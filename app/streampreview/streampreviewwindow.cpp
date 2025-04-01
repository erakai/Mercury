// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "streampreviewwindow.h"
#include "screenlistmodel.h"
#include "windowlistmodel.h"
#include "audioinputlistmodel.h"
#include "../singleton/videomanager.h"
#include "singleton/audiomanager.hpp"

#include <QMediaCaptureSession>
#include <QScreenCapture>
#include <QVideoWidget>

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QAction>
#include <QCheckBox>

StreamPreviewWindow::StreamPreviewWindow(QWidget *parent)
    : QWidget(parent), screenListView(new QListView(this)),
      windowListView(new QListView(this)),
      audioInputListView(new QListView(this)),
      screenCapture(new QScreenCapture()), windowCapture(new QWindowCapture()),
      audioCapture(new QAudioInput(this)),
      mediaCaptureSession(new QMediaCaptureSession()),
      videoWidget(new QVideoWidget(this)), gridLayout(new QGridLayout(this)),
      startStopButton(new QPushButton(this)),
      screenLabel(new QLabel(tr("Select screen to capture:"), this)),
      windowLabel(new QLabel(tr("Select window to capture:"), this)),
      audioInputLabel(new QLabel(tr("Select audio input source"))),
      videoWidgetLabel(new QLabel(tr("Capture output:"), this)),
      captureWindowAudioButton(new QCheckBox(this))
{
  setAttribute(Qt::WA_DeleteOnClose, false);

  // Get lists of screens and windows:
  screenListModel = new ScreenListModel(this);
  windowListModel = new WindowListModel(this);
  audioInputListModel = new AudioInputListModel(this);

  // Setup QScreenCapture with initial source:

  mediaCaptureSession->setScreenCapture(screenCapture);
  mediaCaptureSession->setWindowCapture(windowCapture);
  mediaCaptureSession->setAudioInput(audioCapture);
  mediaCaptureSession->setVideoOutput(videoWidget);

  // Setup UI:

  screenListView->setModel(screenListModel);
  windowListView->setModel(windowListModel);
  audioInputListView->setModel(audioInputListModel);

  auto updateAction = new QAction(tr("Update Windows List"), this);
  connect(updateAction, &QAction::triggered, windowListModel,
          &WindowListModel::populate);
  windowListView->addAction(updateAction);
  windowListView->setContextMenuPolicy(Qt::ActionsContextMenu);

  captureWindowAudioButton->setText(tr("Capture audio for this source"));
  captureWindowAudioButton->setChecked(true);

  startStopButton->setText(tr("Start Stream"));

  gridLayout->addWidget(screenLabel, 0, 0);
  gridLayout->addWidget(screenListView, 1, 0);
  gridLayout->addWidget(windowLabel, 2, 0);
  gridLayout->addWidget(windowListView, 3, 0);
  gridLayout->addWidget(startStopButton, 4, 0);
  gridLayout->addWidget(videoWidgetLabel, 0, 1);
  gridLayout->addWidget(videoWidget, 1, 1, 4, 1);
  gridLayout->addWidget(captureWindowAudioButton, 5, 1);
  gridLayout->addWidget(audioInputLabel, 5, 0);
  gridLayout->addWidget(audioInputListView, 6, 0);

  gridLayout->setColumnStretch(1, 1);
  gridLayout->setRowStretch(1, 1);
  gridLayout->setColumnMinimumWidth(0, 400);
  gridLayout->setColumnMinimumWidth(1, 400);
  gridLayout->setRowMinimumHeight(3, 1);

  connect(screenListView->selectionModel(),
          &QItemSelectionModel::selectionChanged, this,
          &StreamPreviewWindow::onCurrentScreenSelectionChanged);
  connect(windowListView->selectionModel(),
          &QItemSelectionModel::selectionChanged, this,
          &StreamPreviewWindow::onCurrentWindowSelectionChanged);
  connect(audioInputListView->selectionModel(),
          &QItemSelectionModel::selectionChanged, this,
          &StreamPreviewWindow::onCurrentAudioInputSelectionChanged);
  connect(startStopButton, &QPushButton::clicked, this,
          &StreamPreviewWindow::onStartStopButtonClicked);
  connect(screenCapture, &QScreenCapture::errorChanged, this,
          &StreamPreviewWindow::onScreenCaptureErrorChanged,
          Qt::QueuedConnection);
  connect(windowCapture, &QWindowCapture::errorChanged, this,
          &StreamPreviewWindow::onWindowCaptureErrorChanged,
          Qt::QueuedConnection);

  updateActive(SourceType::Screen, true);
}

StreamPreviewWindow::~StreamPreviewWindow() = default;

void StreamPreviewWindow::closeEvent(QCloseEvent *event)
{
  emit closed();
  QWidget::closeEvent(event);
}

void StreamPreviewWindow::onCurrentScreenSelectionChanged(
    QItemSelection selection)
{
  if (auto indexes = selection.indexes(); !indexes.empty())
  {
    screenCapture->setScreen(screenListModel->screen(indexes.front()));
    windowCapture->setWindow(QCapturableWindow());
    updateActive(SourceType::Screen, isActive());

    windowListView->clearSelection();
  }
  else
  {
    screenCapture->setScreen(nullptr);
  }
}

void StreamPreviewWindow::onCurrentWindowSelectionChanged(
    QItemSelection selection)
{
  if (auto indexes = selection.indexes(); !indexes.empty())
  {
    auto window = windowListModel->window(indexes.front());
    if (!window.isValid())
    {
      const auto questionResult = QMessageBox::question(
          this, tr("Invalid window"),
          tr("The window is no longer valid. Update the list of windows?"));
      if (questionResult == QMessageBox::Yes)
      {
        updateActive(SourceType::Window, false);

        windowListView->clearSelection();
        windowListModel->populate();
        return;
      }
    }

    windowCapture->setWindow(window);
    screenCapture->setScreen(nullptr);
    updateActive(SourceType::Window, isActive());

    screenListView->clearSelection();
  }
  else
  {
    windowCapture->setWindow({});
  }
}

void StreamPreviewWindow::onCurrentAudioInputSelectionChanged(
    QItemSelection selection)
{
  if (auto indexes = selection.indexes(); !indexes.empty())
  {
    auto audio = audioInputListModel->audioDevice(indexes.front());

    audioCapture->setDevice(audio);
  }
  else
  {
    audioCapture->setDevice({});
  }
}

void StreamPreviewWindow::onWindowCaptureErrorChanged()
{
  if (windowCapture->error() == QWindowCapture::NoError)
    return;

  QMessageBox::warning(this, tr("QWindowCapture: Error occurred"),
                       windowCapture->errorString());
}

void StreamPreviewWindow::onScreenCaptureErrorChanged()
{
  if (screenCapture->error() == QScreenCapture::NoError)
    return;

  QMessageBox::warning(this, tr("QScreenCapture: Error occurred"),
                       screenCapture->errorString());
}

void StreamPreviewWindow::onStartStopButtonClicked()
{
  VideoManager::instance().setMediaCaptureSession(*mediaCaptureSession);
  QAudioDevice audio_device = audioCapture->device();
  AudioManager::instance().set_audio_device(audio_device);
  close();
}

void StreamPreviewWindow::updateActive(SourceType sourceType, bool active)
{
  this->sourceType = sourceType;

  screenCapture->setActive(active && sourceType == SourceType::Screen);
  windowCapture->setActive(active && sourceType == SourceType::Window);
}

bool StreamPreviewWindow::isActive() const
{
  switch (sourceType)
  {
  case SourceType::Window:
    return windowCapture->isActive();
  case SourceType::Screen:
    return screenCapture->isActive();
  default:
    return false;
  }
}

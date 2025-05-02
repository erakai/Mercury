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
#include <QtMultimedia/qaudiooutput.h>
#include <QtMultimedia/qmediaplayer.h>

StreamPreviewWindow::StreamPreviewWindow(QWidget *parent,
                                         QString startButtonName)
    : QWidget(parent), screenListView(new QListView(this)),
      windowListView(new QListView(this)),
      audioInputListView(new QListView(this)),
      screenCapture(new QScreenCapture()), windowCapture(new QWindowCapture()),
      audioCapture(new QAudioInput()),
      mediaCaptureSession(new QMediaCaptureSession()),
      mediaPlayer(new QMediaPlayer()), videoWidget(new QVideoWidget(this)),
      gridLayout(new QGridLayout(this)), startStopButton(new QPushButton(this)),
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

  // Placeholder for audio file:
  audioPlaceholder = new QLabel(this);
  audioPlaceholder->setAlignment(Qt::AlignCenter);
  audioPlaceholder->setPixmap(
      QPixmap("assets/audio_waveform.png")
          .scaled(300, 300, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  audioPlaceholder->hide();

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

  startStopButton->setText(startButtonName);

  fileSelectionLabel =
      new QLabel(tr("Select video/audio file to stream:"), this);
  fileSelectionButton = new QPushButton(tr("Choose File..."), this);

  gridLayout->addWidget(screenLabel, 0, 0);
  gridLayout->addWidget(screenListView, 1, 0);
  gridLayout->addWidget(windowLabel, 2, 0);
  gridLayout->addWidget(windowListView, 3, 0);
  gridLayout->addWidget(fileSelectionLabel, 4, 0);
  gridLayout->addWidget(fileSelectionButton, 5, 0);
  gridLayout->addWidget(videoWidgetLabel, 0, 1);
  gridLayout->addWidget(audioPlaceholder, 1, 1, 4, 1);
  gridLayout->addWidget(videoWidget, 1, 1, 4, 1);
  gridLayout->addWidget(captureWindowAudioButton, 5, 1);
  gridLayout->addWidget(audioInputLabel, 6, 0);
  gridLayout->addWidget(audioInputListView, 7, 0);
  gridLayout->addWidget(startStopButton, 8, 0);

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
  connect(fileSelectionButton, &QPushButton::clicked, this,
          &StreamPreviewWindow::onCurrentFileMediaChanged);

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
    mediaCaptureSession->setVideoOutput(videoWidget);
    mediaPlayer->setVideoOutput(nullptr);
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

    mediaCaptureSession->setVideoOutput(videoWidget);
    mediaPlayer->setVideoOutput(nullptr);

    windowCapture->setWindow(window);
    screenCapture->setScreen(nullptr);
    updateActive(SourceType::Window, true);

    screenListView->clearSelection();
  }
  else
  {
    windowCapture->setWindow({});
  }
}

void StreamPreviewWindow::onCurrentFileMediaChanged()
{
  QString filePath = QFileDialog::getOpenFileName(
      this, tr("Select Video/Audio File"), "",
      tr("Media Files (*.mp4 *.mp3 *.wav *.avi *.mkv);;All Files (*)"));
  if (!filePath.isEmpty())
  {
    // Determine if it's audio or video based on extension
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();

    QStringList videoFormats = {"mp4", "avi", "mkv", "mov"};
    QStringList audioFormats = {"mp3", "wav", "aac", "flac"};

    if (videoFormats.contains(suffix))
    {
      mediaCaptureSession->setVideoOutput(nullptr);
      mediaPlayer->setVideoOutput(videoWidget);
      updateActive(SourceType::File, isActive());

      audioPlaceholder->hide();
      videoWidget->show();

      mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
      mediaPlayer->play();
      mediaPlayer->pause();
    }
    else if (audioFormats.contains(suffix))
    {
      mediaCaptureSession->setVideoOutput(nullptr);
      mediaPlayer->setVideoOutput(nullptr);
      updateActive(SourceType::File, isActive());

      mediaPlayer->setSource(QUrl::fromLocalFile(filePath));
      videoWidget->hide();
      audioPlaceholder->show();
    }
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
  // start recording audio of host
  if (AudioManager::instance().is_recording())
  {
    AudioManager::instance().stop_recording();
  }
  mediaPlayer->stop();

  if (sourceType == SourceType::Screen || sourceType == SourceType::Window)
  {
    VideoManager::instance().setMediaCaptureSession(*mediaCaptureSession);
    QAudioDevice audio_device = audioCapture->device();
    AudioManager::instance().set_audio_device(audio_device);
  }
  else if (sourceType == SourceType::File)
  {
    mediaPlayer->setVideoOutput(nullptr);
    VideoManager::instance().setMediaPlayer(*mediaPlayer);
    AudioManager::instance().set_media_player(*mediaPlayer);
    mediaPlayer->play();
  }

  if (captureWindowAudioButton->isChecked())
    AudioManager::instance().start_recording(3000);

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
  case SourceType::File:
    return false;
  default:
    return false;
  }
}

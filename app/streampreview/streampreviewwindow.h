// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#ifndef SCREENCAPTUREPREVIEW_H
#define SCREENCAPTUREPREVIEW_H

#include <QScreenCapture>
#include <QWindowCapture>
#include <QAudioInput>
#include <QWidget>
#include <QItemSelection>

class ScreenListModel;
class WindowListModel;
class AudioInputListModel;

QT_BEGIN_NAMESPACE
class QListView;
class QMediaCaptureSession;
class QVideoWidget;
class QGridLayout;
class QHBoxLayout;
class QLineEdit;
class QPushButton;
class QLabel;
class QCheckBox;
QT_END_NAMESPACE

QT_USE_NAMESPACE

class StreamPreviewWindow : public QWidget
{
  Q_OBJECT

public:
  explicit StreamPreviewWindow(QWidget *parent = nullptr);
  ~StreamPreviewWindow() override;

protected:
  void closeEvent(QCloseEvent *event) override;

private slots:
  void onCurrentScreenSelectionChanged(QItemSelection index);
  void onCurrentWindowSelectionChanged(QItemSelection index);
  void onCurrentAudioInputSelectionChanged(QItemSelection index);
  void onWindowCaptureErrorChanged();
  void onScreenCaptureErrorChanged();
  void onStartStopButtonClicked();

signals:
  void closed();

private:
  enum class SourceType
  {
    Screen,
    Window
  };

  void updateActive(SourceType sourceType, bool active);
  bool isActive() const;

private:
  ScreenListModel *screenListModel = nullptr;
  WindowListModel *windowListModel = nullptr;
  AudioInputListModel *audioInputListModel = nullptr;
  QListView *screenListView = nullptr;
  QListView *windowListView = nullptr;
  QListView *audioInputListView = nullptr;
  QScreenCapture *screenCapture = nullptr;
  QWindowCapture *windowCapture = nullptr;
  QAudioInput *audioCapture = nullptr;
  QMediaCaptureSession *mediaCaptureSession = nullptr;
  QVideoWidget *videoWidget = nullptr;
  QGridLayout *gridLayout = nullptr;
  QPushButton *startStopButton = nullptr;
  QLabel *screenLabel = nullptr;
  QLabel *windowLabel = nullptr;
  QLabel *audioInputLabel = nullptr;
  QLabel *videoWidgetLabel = nullptr;
  QCheckBox *captureWindowAudioButton = nullptr;
  SourceType sourceType = SourceType::Screen;
};

#endif // SCREENCAPTUREPREVIEW_H

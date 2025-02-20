#pragma once

#include <QAudioOutput>
#include <QPainter>
#include <QWidget>

class StreamDisplay : public QWidget
{
  Q_OBJECT;

public:
  StreamDisplay(QWidget *parent) : QWidget(parent) {}

private:
  // I think that audio will have to somehow be written to a QIODevice that a
  // QMediaPlayer reads from.
  // https://stackoverflow.com/questions/4473608/how-to-play-sound-with-qt

  // QMediaPlayer *audio;
  // QPainter *painter;
};

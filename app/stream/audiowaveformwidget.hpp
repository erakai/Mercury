#pragma once

#include <QWidget>
#include <QByteArray>
#include <QTimer>

class AudioWaveformWidget : public QWidget
{
  Q_OBJECT

public:
  explicit AudioWaveformWidget(QWidget *parent = nullptr);

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void updateWaveform();

private:
  QByteArray audioData;
  QTimer timer;

  float autoGainFactor(const QByteArray &data) const;
};

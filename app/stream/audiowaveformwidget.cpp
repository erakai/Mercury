#include "audiowaveformwidget.hpp"
#include "singleton/audiomanager.hpp"
#include <QPainter>

AudioWaveformWidget::AudioWaveformWidget(QWidget *parent) : QWidget(parent)
{
  connect(&timer, &QTimer::timeout, this, &AudioWaveformWidget::updateWaveform);
  timer.start(50);
}

float AudioWaveformWidget::autoGainFactor(const QByteArray &data) const
{
  if (data.isEmpty())
    return 1.0f;

  float sumAbs = 0;
  for (int i = 0; i < data.size(); ++i)
  {
    sumAbs += std::abs(static_cast<qint8>(data[i]));
  }
  float avgAbs = sumAbs / data.size();

  // Avoid division by zero and set a baseline gain
  if (avgAbs < 5)
    avgAbs = 5;

  float targetLevel =
      64.0f; // Aim to scale average amplitude to about half the range
  return targetLevel / avgAbs;
}

void AudioWaveformWidget::paintEvent(QPaintEvent *event)
{
  Q_UNUSED(event);

  QPainter painter(this);
  painter.fillRect(rect(), QColor("#0B1114"));

  if (audioData.isEmpty())
    return;

  painter.setPen(QColor("#087AA0"));

  int width = this->width();
  int height = this->height();
  int midY = height / 2;

  const int sampleCount = audioData.size();
  if (sampleCount == 0)
    return;

  float samplesPerPixel = (float) sampleCount / width;
  float gain = autoGainFactor(audioData);
  float offset = 4;

  QPoint lastPoint(0, midY);
  for (int x = 0; x < width; ++x)
  {
    int startIdx = x * samplesPerPixel;
    int endIdx = std::min((int) ((x + 1) * samplesPerPixel), sampleCount);
    if (startIdx >= endIdx)
      continue;

    int sum = 0;
    for (int i = startIdx; i < endIdx; ++i)
    {
      sum += static_cast<qint8>(audioData[i]);
    }
    float avg = (float) sum / (endIdx - startIdx);

    int y = midY - (avg * gain * offset / 128.0) * ((float) height / 2);

    painter.drawLine(lastPoint, QPoint(x, y));
    lastPoint = QPoint(x, y);
  }
}

void AudioWaveformWidget::updateWaveform()
{
  audioData = AudioManager::instance().get_lastmsec(500);
  update();
}

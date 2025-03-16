#pragma once

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <vector>
#include <QPoint>

class AnnotationDisplay : public QWidget
{
  Q_OBJECT
public:
  explicit AnnotationDisplay(QWidget *parent = nullptr) : QWidget(parent)
  {
    // Make sure the background is transparent.
    setAttribute(Qt::WA_TranslucentBackground);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  }

  // Container to store annotation points.
  std::vector<QPoint> points;

protected:
  void paintEvent(QPaintEvent *event) override
  {
    qDebug() << "paintEvent" << width() << " " << height();
    // QWidget::paintEvent(event);
    QPainter painter(this);
    QPen pen(Qt::red);
    pen.setWidth(2);
    painter.setPen(pen);

    if (points.size() > 1)
    {
      for (size_t i = 1; i < points.size(); ++i)
      {
        qDebug() << "points[" << i << "]";
        painter.drawLine(points[i - 1], points[i]);
      }
    }
  }
};

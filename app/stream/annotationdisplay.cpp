#include "annotationdisplay.hpp"
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <QResizeEvent>

AnnotationDisplay::AnnotationDisplay(QWidget *parent) : QWidget(parent)
{
  // Ensure the widget's background is transparent.
  setAttribute(Qt::WA_TranslucentBackground);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Initialize the pixmap with the current widget size and fill it transparent.
  m_pixmap = QPixmap(size());
  m_pixmap.fill(Qt::transparent);
}

void AnnotationDisplay::addLine(const QPoint &start, const QPoint &end,
                                QColor color, int thickness)
{
  qDebug() << "Drawing line persistently from" << start << "to" << end;
  // Paint the new line onto the pixmap.
  QPainter painter(&m_pixmap);
  QPen pen(color);
  pen.setWidth(thickness);
  painter.setPen(pen);
  painter.drawLine(start, end);

  // Request an update so that paintEvent is called.
  update();
}

void AnnotationDisplay::paintEvent(QPaintEvent *event)
{
  QPainter painter(this);
  // Simply draw the entire pixmap.
  painter.drawPixmap(0, 0, m_pixmap);
}

void AnnotationDisplay::resizeEvent(QResizeEvent *event)
{
  // When the widget is resized, create a new pixmap of the appropriate size.
  if (event->size() != m_pixmap.size())
  {
    QPixmap newPixmap(event->size());
    newPixmap.fill(Qt::transparent);

    // Copy the content of the old pixmap to the new one.
    QPainter p(&newPixmap);
    p.drawPixmap(0, 0, m_pixmap);
    m_pixmap = newPixmap;
  }
  QWidget::resizeEvent(event);
}

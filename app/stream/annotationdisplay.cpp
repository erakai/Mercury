#include "annotationdisplay.hpp"
#include "painttoolwidget.hpp"

#include <QPainter>
#include <QPen>
#include <QDebug>
#include <QResizeEvent>
#include <QVBoxLayout>

AnnotationDisplay::AnnotationDisplay(QWidget *parent) : QWidget(parent)
{
  // Ensure the widget's background is transparent.
  setAttribute(Qt::WA_TranslucentBackground);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // Initialize the pixmap with the current widget size and fill it transparent.
  m_pixmap = QPixmap(size());
  m_pixmap.fill(Qt::transparent);

  // Create a vertical layout for this widget.
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(0, 0, 0, 0);

  // Add a stretch to push remaining content (your drawing) to fill the rest of
  // the area.
  mainLayout->addStretch();
}

void AnnotationDisplay::addLine(const QPoint &start, const QPoint &end,
                                QColor color, int thickness)
{
  // qDebug() << "Drawing line persistently from" << start << "to" << end;
  if (!canAnnotate)
    return;

  QPainter painter(&m_pixmap);
  QPen pen(color);

  if (thickness < 0)
  {
    // Erasing mode: use the absolute thickness and set composition mode to
    // clear.
    pen.setWidth(-thickness);
    painter.setCompositionMode(QPainter::CompositionMode_Clear);
  }
  else
  {
    pen.setWidth(thickness);
  }

  painter.setPen(pen);
  painter.drawLine(start, end);

  // Request an update so that paintEvent is called.
  update();
}

void AnnotationDisplay::clear()
{
  m_pixmap.fill(Qt::transparent);
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

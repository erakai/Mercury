// annotationdisplay.cpp
#include "annotationdisplay.hpp"
#include <QPainter>
#include <QPen>

AnnotationDisplay::AnnotationDisplay(QWidget *parent) : QWidget(parent)
{
  setAttribute(Qt::WA_TranslucentBackground);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

  // leave m_pixmap null until the first real resizeEvent
  // (so it picks up the true widget size)
}

// annotationdisplay.cpp

void AnnotationDisplay::addLine(const QPoint &start, const QPoint &end,
                                QColor color, int thickness, int mode)
{
  if (!canAnnotate || m_pixmap.isNull() || width() <= 0 || height() <= 0)
    return;

  // map widget coords → master‐pixmap coords
  const double xScale = double(m_pixmap.width()) / double(width());
  const double yScale = double(m_pixmap.height()) / double(height());
  QPoint masterStart(int(start.x() * xScale), int(start.y() * yScale));
  QPoint masterEnd(int(end.x() * xScale), int(end.y() * yScale));

  QPainter p(&m_pixmap);
  p.setRenderHint(QPainter::Antialiasing);
  QPen pen;

  if (thickness < 0)
  {
    // eraser
    pen.setWidth(-thickness);
    p.setCompositionMode(QPainter::CompositionMode_Clear);
  }
  else
  {
    switch (mode)
    {
    case 1: // brush: thicker, round caps
      pen.setColor(color);
      pen.setWidth(thickness * 2);
      pen.setCapStyle(Qt::RoundCap);
      pen.setJoinStyle(Qt::RoundJoin);
      break;
    case 2: // highlighter: semi-transparent, broad
    {
      QColor hl = color;
      hl.setAlpha(60);
      pen.setColor(hl);
      pen.setWidth(thickness * 4);
      break;
    }
    case 0: // pen
    default:
      pen.setColor(color);
      pen.setWidth(thickness);
      break;
    }
  }

  p.setPen(pen);
  p.drawLine(masterStart, masterEnd);

  updateScaled();
}

void AnnotationDisplay::clear()
{
  if (!m_pixmap.isNull())
  {
    m_pixmap.fill(Qt::transparent);
    updateScaled();
  }
}

void AnnotationDisplay::resizeEvent(QResizeEvent *e)
{
  QWidget::resizeEvent(e);
  const QSize newSize = e->size();

  if (newSize.width() <= 0 || newSize.height() <= 0)
    return;

  // 1) first time through: allocate master at the right size
  if (m_pixmap.isNull())
  {
    m_pixmap = QPixmap(newSize);
    m_pixmap.fill(Qt::transparent);
  }
  // 2) if the widget grew beyond our master, expand it (preserving old content)
  else if (newSize.width() > m_pixmap.width() ||
           newSize.height() > m_pixmap.height())
  {
    // new master that’s at least as big as both
    QPixmap bigger = QPixmap(newSize.expandedTo(m_pixmap.size()));
    bigger.fill(Qt::transparent);

    QPainter p(&bigger);
    // copy your old annotations into the top-left
    p.drawPixmap(0, 0, m_pixmap);

    m_pixmap = std::move(bigger);
  }

  // in all cases, rebuild the scaled view
  updateScaled();
}

void AnnotationDisplay::updateScaled()
{
  const QSize sz = size();
  if (sz.width() <= 0 || sz.height() <= 0 || m_pixmap.isNull())
    return;

  // one smooth resample per resize or draw
  m_scaled =
      m_pixmap.scaled(sz, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  // trigger paintEvent
  update();
}

void AnnotationDisplay::paintEvent(QPaintEvent *)
{
  if (m_scaled.isNull())
    return;

  QPainter painter(this);
  // smooth it again at draw time
  painter.setRenderHint(QPainter::SmoothPixmapTransform);
  painter.drawPixmap(0, 0, m_scaled);
}

#ifndef HOVERLABEL_H
#define HOVERLABEL_H

#include <QLabel>
#include <QPainter>
#include <QEvent>

class HoverLabel : public QLabel {
  Q_OBJECT
public:
  explicit HoverLabel(QWidget *parent = nullptr)
      : QLabel(parent), m_hovered(false)
  {
    // Enable hover events so that we receive QEvent::HoverEnter/Leave.
    setAttribute(Qt::WA_Hover, true);
  }

         // Set the text that should appear when the label is hovered.
  void setFlavorText(const QString &text) { m_flavorText = text; }

protected:
  // Override the event() function to capture hover events.
  bool event(QEvent *e) override {
    if (e->type() == QEvent::HoverEnter) {
      m_hovered = true;
      update();  // Trigger repaint to show outline and text.
    } else if (e->type() == QEvent::HoverLeave) {
      m_hovered = false;
      update();
    }
    return QLabel::event(e);
  }

         // Override paintEvent() to draw an outline and flavor text when hovered.
  void paintEvent(QPaintEvent *event) override {
    // Let the base class draw the pixmap (or text) normally.
    QLabel::paintEvent(event);

    if (m_hovered) {
      QPainter painter(this);

             // Draw a blue outline.
      QPen pen(Qt::blue);
      pen.setWidth(2);
      painter.setPen(pen);
      painter.drawRect(rect().adjusted(1, 1, -1, -1));

             // If flavor text is set, draw it on a semi-transparent background.
      if (!m_flavorText.isEmpty()) {
        QRect textRect = rect();
        textRect.setTop(textRect.bottom() - 20);
        painter.fillRect(textRect, QColor(0, 0, 0, 128)); // semi-transparent black
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(textRect, Qt::AlignCenter, m_flavorText);
      }
    }
  }

private:
  bool m_hovered;
  QString m_flavorText;
};

#endif // HOVERLABEL_H

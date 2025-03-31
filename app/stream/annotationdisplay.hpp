#pragma once

#include "painttoolwidget.hpp"

#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <QPoint>

class AnnotationDisplay : public QWidget
{
  Q_OBJECT
public:
  explicit AnnotationDisplay(QWidget *parent = nullptr);

  // Add a line persistently from start to end with the specified color and
  // thickness.
  void addLine(const QPoint &start, const QPoint &end, QColor color = Qt::red,
               int thickness = 2);

  PaintToolWidget *paint_tool_widget;

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  // Off-screen pixmap that holds all drawn annotations.
  QPixmap m_pixmap;
};

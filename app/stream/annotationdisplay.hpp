#pragma once

#include <QWidget>
#include <QPixmap>
#include <QColor>
#include <QPoint>
#include <QPaintEvent>
#include <QResizeEvent>

class AnnotationDisplay : public QWidget
{
  Q_OBJECT
public:
  explicit AnnotationDisplay(QWidget *parent = nullptr);

  // Add a line persistently from start to end with the specified color and
  // thickness.
  void addLine(const QPoint &start, const QPoint &end, QColor color = Qt::red,
               int thickness = 3, int mode = 0);

  void clear();

  bool canAnnotate = true;

protected:
  void paintEvent(QPaintEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  // Off-screen pixmap that holds all drawn annotations.
  QPixmap m_pixmap;

  QPixmap m_scaled;
  void updateScaled();
};

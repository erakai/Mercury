#ifndef PAINTTOOLWIDGET_HPP
#define PAINTTOOLWIDGET_HPP

#include <QWidget>
#include <QColor>
#include <QLineEdit>
#include <QRadioButton>

class QLabel;
class QPushButton;

class PaintToolWidget : public QWidget
{
  Q_OBJECT
public:
  explicit PaintToolWidget(QWidget *parent = nullptr);

  // Getter for the currently selected color.
  QColor selectedColor() const { return m_selectedColor; }
  int brushSize() const { return m_brushSizeEdit->text().toInt(); }
  bool isEraseMode() const { return m_eraseButton->isChecked(); }
  void addClearButton();

signals:
  void clearButtonClicked();

private:
  QColor m_selectedColor;
  QLabel *m_colorIndicator;
  QPushButton *m_colorButton;
  QLineEdit *m_brushSizeEdit;
  QRadioButton *m_eraseButton;
};

#endif // PAINTTOOLWIDGET_HPP

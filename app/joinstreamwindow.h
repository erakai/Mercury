#ifndef JOINSTREAMWINDOW_H
#define JOINSTREAMWINDOW_H

#include <QDialog>

namespace Ui
{
class JoinStreamWindow;
}

class JoinStreamWindow : public QDialog
{
  Q_OBJECT

public:
  explicit JoinStreamWindow(QWidget *parent = nullptr);
  ~JoinStreamWindow();

private slots:
  void on_cancelButton_clicked();

private:
  Ui::JoinStreamWindow *ui;
};

#endif // JOINSTREAMWINDOW_H

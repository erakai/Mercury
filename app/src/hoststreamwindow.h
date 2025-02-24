#ifndef HOSTSTREAMWINDOW_H
#define HOSTSTREAMWINDOW_H

#include <QDialog>

namespace Ui
{
class HostStreamWindow;
}

class HostStreamWindow : public QDialog
{
  Q_OBJECT

public:
  explicit HostStreamWindow(QWidget *parent = nullptr);
  ~HostStreamWindow();

private slots:
  void on_pushButton_clicked();

  void on_ip_address_button_clicked();

private:
  Ui::HostStreamWindow *ui;
};

#endif // HOSTSTREAMWINDOW_H

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
  void on_hostButton_clicked();

  void on_ipAddressButton_clicked();

  void on_cancelButton_clicked();

private:
  Ui::HostStreamWindow *ui;
};

#endif // HOSTSTREAMWINDOW_H

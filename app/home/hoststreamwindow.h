#ifndef HOSTSTREAMWINDOW_H
#define HOSTSTREAMWINDOW_H

#include "streampreview/streampreviewwindow.h"
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

  void open_stream_window();
  void on_ipAddressButton_clicked();
  void on_cancelButton_clicked();
  void tutorial_button_press();
  void public_stream_toggled();

  void on_ethics_button_clicked();

private:
  void set_ethics_agreed_to(bool agreed);
  bool ethics_agreed_to;

  Ui::HostStreamWindow *ui;

  StreamPreviewWindow *spw = nullptr;
};

#endif // HOSTSTREAMWINDOW_H

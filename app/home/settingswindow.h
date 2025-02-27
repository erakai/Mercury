#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QDialog>

namespace Ui
{
class SettingsWindow;
}

class SettingsWindow : public QDialog
{
  Q_OBJECT

public:
  explicit SettingsWindow(QWidget *parent = nullptr);
  ~SettingsWindow();

signals:
  void aliasChanged(const QString &alias);

private slots:
  void on_closeButton_clicked();

  void on_applyButton_clicked();

private:
  Ui::SettingsWindow *ui;
  void set_button_ids();
  void set_up();
};

#endif // SETTINGSWINDOW_H

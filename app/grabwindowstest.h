#ifndef GRABWINDOWSTEST_H
#define GRABWINDOWSTEST_H

#include <QDialog>

namespace Ui
{
class grabwindowstest;
}

class grabwindowstest : public QDialog
{
  Q_OBJECT

public:
  explicit grabwindowstest(QWidget *parent = nullptr);
  ~grabwindowstest();

private:
  Ui::grabwindowstest *ui;
};

#endif // GRABWINDOWSTEST_H

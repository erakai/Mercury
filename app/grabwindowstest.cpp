#include "grabwindowstest.h"
#include "ui_grabwindowstest.h"

grabwindowstest::grabwindowstest(QWidget *parent)
    : QDialog(parent), ui(new Ui::grabwindowstest)
{
  ui->setupUi(this);
}

grabwindowstest::~grabwindowstest()
{
  delete ui;
}

#include "grabwindowstest.h"
#include "ui_grabwindowstest.h"
#include "windowmanager.h"
#include "hoverlabel.h"
#include <QLabel>

grabwindowstest::grabwindowstest(QWidget *parent)
    : QDialog(parent), ui(new Ui::grabwindowstest)
{
  ui->setupUi(this);
}

grabwindowstest::~grabwindowstest()
{
  delete ui;
}

void grabwindowstest::on_button_grab_windows_clicked()
{
  // Persist the image count between clicks.
  static int imageCount = 0;
  const int columns = 3; // Adjust the number of columns as needed

  std::pair<QList<QPixmap>, QList<std::string>> windows =
      WindowManager::instance().grab_pixmaps_of_active_windows();
  for (int i = 0; i < windows.first.size(); i++)
  {
    // Create a new HoverLabel for each image.
    HoverLabel *imageLabel = new HoverLabel(this);
    imageLabel->setPixmap(windows.first[i]);
    imageLabel->setScaledContents(true);

    // Set the flavor text. You can customize this as needed.
    imageLabel->setFlavorText(QString::fromStdString(windows.second[i]));

    // Calculate the grid position based on imageCount.
    int row = imageCount / columns;
    int col = imageCount % columns;
    ui->gridLayout->addWidget(imageLabel, row, col);

    ++imageCount;
  }
}

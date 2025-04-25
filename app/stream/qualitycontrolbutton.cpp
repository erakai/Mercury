#include "qualitycontrolbutton.hpp"

QualityControlButton::QualityControlButton(QWidget *parent)
    : QPushButton(parent)
{

  QIcon gear_icon = QIcon::fromTheme("emblem-system");
  setIcon(gear_icon);
  setIconSize(QSize(32, 32));

  QMenu *quality_menu = new QMenu(this);

  option0 = new QAction("Auto", this);
  option1 = new QAction("240p", this);
  option2 = new QAction("480p", this);
  option3 = new QAction("720p", this);
  option4 = new QAction("1080p", this);

  option0->setCheckable(true);
  option1->setCheckable(true);
  option2->setCheckable(true);
  option3->setCheckable(true);
  option4->setCheckable(true);

  option0->setChecked(true);

  quality_menu->addAction(option0);
  quality_menu->addAction(option1);
  quality_menu->addAction(option2);
  quality_menu->addAction(option3);
  quality_menu->addAction(option4);
  setMenu(quality_menu);

  connect(option0, &QAction::triggered, this,
          [=, this]()
          {
            uncheck_all();
            option0->setChecked(true);
            emit quality_changed(QualityOption::DEFAULT);
          });
  connect(option1, &QAction::triggered, this,
          [=, this]()
          {
            uncheck_all();
            option1->setChecked(true);
            emit quality_changed(QualityOption::Q240);
          });
  connect(option2, &QAction::triggered, this,
          [=, this]()
          {
            uncheck_all();
            option2->setChecked(true);
            emit quality_changed(QualityOption::Q480);
          });
  connect(option3, &QAction::triggered, this,
          [=, this]()
          {
            uncheck_all();
            option3->setChecked(true);
            emit quality_changed(QualityOption::Q720);
          });
  connect(option4, &QAction::triggered, this,
          [=, this]()
          {
            uncheck_all();
            option4->setChecked(true);
            emit quality_changed(QualityOption::Q1080);
          });

  // Styling
  setStyleSheet("QPushButton {"
                "text-align: left;"
                "padding: 10px;"
                "border: none;"
                "background-color: transparent;"
                "}"
                "QPushButton::menu-indicator {"
                "width:0px;"
                "}");

  quality_menu->setStyleSheet(
      "QMenu {"
      "background-color: #2e2e2e;"
      "color: #DCEAEE;"
      "border: 1px solid #444;"
      "border-radius: 5px;"
      "padding: 3px;"
      "}"
      "QMenu::item {"
      "padding: 6px 4;"
      "}"
      "QMenu::item:selected {"
      "background-color: #575757;"
      "}"
      "QMenu::item:checked {"
      "background-color: #E39F20;" // Change background for checked item
      "color: #DCEAEE;"
      "}");
}

void QualityControlButton::uncheck_all()
{
  option0->setChecked(false);
  option1->setChecked(false);
  option2->setChecked(false);
  option3->setChecked(false);
  option4->setChecked(false);
}

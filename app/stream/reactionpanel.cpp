#include "reactionpanel.hpp"

#include <QLabel>
#include <QPushButton>
#include <QtWidgets/qgridlayout.h>

const std::vector<std::string> ReactionPanel::reactionStringList = {
    "thumbsup", "heart", "laugh",      "surprised", "fire",
    "sob",      "angry", "thumbsdown", "skull"};

ReactionPanel::ReactionPanel(QWidget *parent)
{
  auto *panel_container = new QWidget(this);
  panel_container->setStyleSheet(
      "QWidget {"
      "    background-color: #222222;" // Dark background
      "    border-radius: 12px;"       // Rounded corners
      "    padding: 10px;"             // Internal spacing
      "}");
  auto *reaction_button_grid = new QGridLayout(parent);

  int index = 0;
  int cols = 4;
  int rows = 3;
  for (int i = 0; i < rows; i++)
  {
    for (int j = 0; j < cols; j++)
    {
      if (index >= reactionStringList.size())
      {
        goto doneAddingButtons;
      }
      auto *reaction_button = new QPushButton(parent);
      // reaction_button->setStyleSheet("");
      auto reaction = static_cast<Reaction>(index);
      QString filename =
          QString("assets/reactions/%1").arg(getReactionAssetName(reaction));
      QPixmap pix(filename);
      reaction_button->setIcon(QIcon(pix));
      reaction_button->setIconSize(QSize(24, 24));
      reaction_button->setFixedSize(40, 40);
      reaction_button->setStyleSheet(
          "QPushButton {"
          "    background-color: #333333;" // Dark gray
          "    border-radius: 20px;"       // Fully rounded
          "    border: none;"              // Remove border
          "}"
          "QPushButton:hover {"
          "    background-color: #444444;" // Slightly lighter on hover
          "}"
          "QPushButton:pressed {"
          "    background-color: #555555;" // Even lighter when pressed
          "}");
      reaction_button_grid->addWidget(reaction_button, i, j);
      connect(reaction_button, &QPushButton::clicked, this,
              [this, reaction]()
              {
                qDebug() << "reaction clicked, emitting reaction";
                emit reactionClicked(reaction);
              });

      index++;
    }
  }
doneAddingButtons:

  panel_container->setLayout(reaction_button_grid);
  auto *main_layout = new QVBoxLayout(this);
  // auto *reaction_panel_label = new QLabel(this);
  // reaction_panel_label->setStyleSheet("color: #dddddd; font-size: 18px;
  // padding: 8px; border-radius: 10px; background-color: #222222;");
  // reaction_panel_label->setSizePolicy(QSizePolicy::Fixed,
  // QSizePolicy::Fixed); reaction_panel_label->setText("Reactions");
  //
  // main_layout->addWidget(reaction_panel_label);
  main_layout->addWidget(panel_container);
  this->setLayout(main_layout);
}

QString ReactionPanel::getReactionAssetName(Reaction reaction)
{
  return "reaction_" +
         QString::fromStdString(
             reactionStringList.at(static_cast<int>(reaction))) +
         ".png";
}
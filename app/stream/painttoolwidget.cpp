#include "painttoolwidget.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QVBoxLayout>

PaintToolWidget::PaintToolWidget(QWidget *parent) : QWidget(parent)
{
  // Create a label and a textbox for the brush size with a validator for
  // numbers 1-100.
  QLabel *brushSizeLabel = new QLabel("Brush Size:");
  QLineEdit *brushSizeEdit = new QLineEdit;
  brushSizeEdit->setValidator(new QIntValidator(1, 100, this));

  // Create a label and a color dropdown menu.
  QLabel *colorLabel = new QLabel("Color:");
  QComboBox *colorComboBox = new QComboBox;
  colorComboBox->addItem("Red");
  colorComboBox->addItem("Green");
  colorComboBox->addItem("Blue");
  colorComboBox->addItem("Black");
  colorComboBox->addItem("White");

  // Create radio buttons for "Brush" and "Erase".
  QRadioButton *brushButton = new QRadioButton("Brush");
  QRadioButton *eraseButton = new QRadioButton("Erase");

  // Group the buttons so that selecting one will unselect the other.
  QButtonGroup *modeGroup = new QButtonGroup(this);
  modeGroup->addButton(brushButton);
  modeGroup->addButton(eraseButton);
  modeGroup->setExclusive(true);

  // Layout for the brush size input.
  QHBoxLayout *brushSizeLayout = new QHBoxLayout;
  brushSizeLayout->addWidget(brushSizeLabel);
  brushSizeLayout->addWidget(brushSizeEdit);

  // Layout for the color dropdown.
  QHBoxLayout *colorLayout = new QHBoxLayout;
  colorLayout->addWidget(colorLabel);
  colorLayout->addWidget(colorComboBox);

  // Layout for the radio buttons.
  QHBoxLayout *buttonsLayout = new QHBoxLayout;
  buttonsLayout->addWidget(brushButton);
  buttonsLayout->addWidget(eraseButton);

  // Main layout combining all the components.
  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(brushSizeLayout);
  mainLayout->addLayout(colorLayout);
  mainLayout->addLayout(buttonsLayout);

  setLayout(mainLayout);
}

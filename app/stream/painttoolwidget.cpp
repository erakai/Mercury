#include "painttoolwidget.hpp"
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QComboBox>

PaintToolWidget::PaintToolWidget(QWidget *parent) : QWidget(parent)
{
  // Create a label and a textbox for the brush size with a validator for
  // numbers 1-100.
  QLabel *brushSizeLabel = new QLabel("Brush Size:");
  m_brushSizeEdit = new QLineEdit;
  m_brushSizeEdit->setText("3");
  m_brushSizeEdit->setFixedWidth(90);
  m_brushSizeEdit->setValidator(new QIntValidator(1, 100, this));

  // Create a small circular label to serve as a color indicator.
  m_colorIndicator = new QLabel;
  m_colorIndicator->setFixedSize(20, 20);
  // Set the default color to white.
  m_selectedColor = Qt::white;
  m_colorIndicator->setStyleSheet("background-color: #FFFFFF; border-radius: "
                                  "10px; border: 1px solid black;");

  // Create a button for selecting a color.
  m_colorButton = new QPushButton("Select Color");

  // Connect the button to open a color dialog.
  connect(m_colorButton, &QPushButton::clicked, this,
          [this]()
          {
            QColor chosenColor =
                QColorDialog::getColor(m_selectedColor, this, "Choose Color");
            if (chosenColor.isValid())
            {
              // Update the stored color.
              m_selectedColor = chosenColor;
              // Get the hexadecimal representation of the color.
              QString hexColor = m_selectedColor.name();
              // Update the color indicator's style to reflect the new color.
              m_colorIndicator->setStyleSheet(
                  QString("background-color: %1; border-radius: 10px; border: "
                          "1px solid black;")
                      .arg(hexColor));
              // Update the button's text to show the hex value.
              m_colorButton->setText(hexColor);
            }
          });

  // Create radio button for "Erase".
  m_eraseButton = new QRadioButton("Erase");

  // Create a combo box for selecting brush type (Brush, Highlighter, Pen).
  m_brushTypeCombo = new QComboBox;
  m_brushTypeCombo->addItem("Pen");
  m_brushTypeCombo->addItem("Brush");
  m_brushTypeCombo->addItem("Highlighter");

  // Set the default selection to "Brush".
  m_brushTypeCombo->setCurrentIndex(0);

  // Group the erase button so it remains exclusive.
  QButtonGroup *modeGroup = new QButtonGroup(this);
  modeGroup->addButton(m_eraseButton);
  modeGroup->setExclusive(true);

  // Create a horizontal layout and add all widgets in order.
  QHBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->addWidget(brushSizeLabel);
  mainLayout->addWidget(m_brushSizeEdit);
  mainLayout->addWidget(m_colorIndicator);
  mainLayout->addWidget(m_colorButton);
  mainLayout->addWidget(m_brushTypeCombo);
  mainLayout->addWidget(m_eraseButton);
  mainLayout->addStretch();

  setLayout(mainLayout);
}

void PaintToolWidget::addClearButton()
{
  // Create the clear button.
  QPushButton *clearButton = new QPushButton("Clear", this);

  // Add the clear button to the existing layout so that it appears on the
  // right.
  layout()->addWidget(clearButton);

  // Connect the clear button's clicked signal to emit the clearButtonClicked
  // signal.
  connect(clearButton, &QPushButton::clicked, this,
          [this]() { emit clearButtonClicked(); });
}

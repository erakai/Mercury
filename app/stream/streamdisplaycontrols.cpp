#include "streamdisplaycontrols.hpp"

#include <QMouseEvent>
#include <QVBoxLayout>
#include <QtCore/QEvent>

StreamDisplayControls::StreamDisplayControls(QWidget *parent)
{

  fullscreenButton = new QPushButton(this);
  fullscreenButton->setFixedSize(36, 36); // Adjust button size as needed
  QPixmap pix("assets/fullscreen_button_icon.png");
  fullscreenButton->setIcon(QIcon(pix));
  fullscreenButton->setIconSize(QSize(36, 36));
  fullscreenButton->setStyleSheet(
      "background-color: rgba(0, 0, 0, 0); border: none;");

  // Layout to align button to bottom-right
  QVBoxLayout *vLayout = new QVBoxLayout();
  vLayout->addStretch(); // Push everything to the bottom
  QHBoxLayout *hLayout = new QHBoxLayout();
  hLayout->addStretch(); // Push button to the right
  hLayout->addWidget(fullscreenButton);
  vLayout->addLayout(hLayout);
  setFixedHeight(fullscreenButton->height());
  // setStyleSheet("border: 2px solid yellow;");
  setContentsMargins(0, 0, 0, 48);
  setLayout(vLayout);

  // Connect button signal
  connect(fullscreenButton, &QPushButton::clicked, this,
          [this]()
          {
            // toggle display mode from fullscreen and default
            emit fullScreenButtonPressed();
          });
}
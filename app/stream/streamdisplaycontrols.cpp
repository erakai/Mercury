#include "streamdisplaycontrols.hpp"

#include "stream/streamwindow.hpp"
#include <QMouseEvent>
#include <QVBoxLayout>
#include <QtCore/QEvent>
#include <QtWidgets/qpushbutton.h>

StreamDisplayControls::StreamDisplayControls(QWidget *parent)
{
  StreamWindow *stream_window = qobject_cast<StreamWindow *>(parent);

  fullscreenButton = new QPushButton(this);
  fullscreenButton->setFixedSize(48, 48); // Adjust button size as needed

  QString app_dir = QCoreApplication::applicationDirPath();
  QString file(app_dir + "/assets/fullscreen_button_icon.png");
  QPixmap pix(file);
  fullscreenButton->setIcon(QIcon(pix));
  fullscreenButton->setIconSize(QSize(48, 48));
  fullscreenButton->setStyleSheet(
      "background-color: rgba(0, 0, 0, 0); border: none;");

  QGridLayout *grid_layout = new QGridLayout(this);

  grid_layout->addWidget(fullscreenButton, 1, 2,
                         Qt::AlignRight | Qt::AlignBottom);

  // Volume Control
  if (stream_window->is_client())
  {
    QSpacerItem *spacer =
        new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

    grid_layout->addItem(spacer, 0, 0); // Spacer in the first row, first column

    volume_control = new VolumeControlWidget(this);
    connect(volume_control, &VolumeControlWidget::volume_changed, this,
            [this](int volume) { emit volume_changed(volume); });
    grid_layout->addWidget(volume_control, 1, 0,
                           Qt::AlignRight | Qt::AlignBottom);

    quality_control = new QualityControlButton(this);
    grid_layout->addWidget(quality_control, 1, 1,
                           Qt::AlignRight | Qt::AlignBottom);
  }

  grid_layout->setColumnStretch(0, 1);
  grid_layout->setColumnStretch(1, 0);
  grid_layout->setRowStretch(0, 1);

  setLayout(grid_layout);

  // Connect button signal
  connect(fullscreenButton, &QPushButton::clicked, this,
          [this]()
          {
            // toggle display mode from fullscreen and default
            emit fullScreenButtonPressed();
          });
}

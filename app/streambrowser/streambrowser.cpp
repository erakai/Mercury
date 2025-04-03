#include "streambrowser.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QEvent>

StreamBrowser::StreamBrowser(QWidget *parent, QStringList streamNames,
                             QList<int> hostTCPs, QStringList streamIPs)
    : QDialog(parent)
{
  setStyleSheet("background-color: transparent;");

  this->streamNames = streamNames;
  this->hostTCPs = hostTCPs;
  this->streamIPs = streamIPs;

  QVBoxLayout *outerLayout = new QVBoxLayout(this);
  outerLayout->setContentsMargins(0, 0, 0, 0);
  outerLayout->setSpacing(0);
  outerLayout->addWidget(createUI());
}

bool StreamBrowser::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
    QWidget *clickedTile = qobject_cast<QWidget *>(watched);
    if (clickedTile)
    {
      // TODO: add connection
      return true;
    }
  }
  return QDialog::eventFilter(watched, event);
}

QWidget *StreamBrowser::createUI()
{
  QWidget *page = new QWidget();
  page->setStyleSheet("background-color: #121619; border-radius: 12px;");

  QVBoxLayout *layout = new QVBoxLayout(page);
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);

  QLabel *topLabel = new QLabel("Stream Browser");
  topLabel->setStyleSheet("color: white; font-size: 40pt; padding-left: 20px;");
  topLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  topLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  layout->addWidget(topLabel, 15);

  QScrollArea *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  scrollArea->setStyleSheet("background-color: transparent; border: none;");

  QWidget *middleBox = new QWidget();
  QGridLayout *grid = new QGridLayout(middleBox);
  grid->setContentsMargins(20, 20, 20, 0);
  grid->setSpacing(20);

  for (int i = 0; i < streamIPs.length(); ++i)
  {
    QWidget *tile = new QWidget();
    tile->setStyleSheet("background-color: #444444; border-radius: 4px;");
    tile->setFixedSize(308, 202);

    QLabel *liveBadge = new QLabel("LIVE", tile);
    liveBadge->move(8, 8);
    liveBadge->setStyleSheet("background-color: #3681a3; color: white; "
                             "padding: 2px 6px; border-radius: 4px;");

    QLabel *label = new QLabel(streamNames[i]);
    label->setStyleSheet("color: white; font-weight: bold;");
    label->setAlignment(Qt::AlignCenter);

    QVBoxLayout *tileLayout = new QVBoxLayout();
    tileLayout->addWidget(tile, 0, Qt::AlignHCenter);
    tileLayout->addWidget(label);
    tileLayout->setAlignment(label, Qt::AlignHCenter);

    QWidget *wrapper = new QWidget();
    wrapper->setLayout(tileLayout);
    wrapper->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    tile->installEventFilter(this);
    tile->setProperty("hostTCP", hostTCPs[i]);
    tile->setProperty("streamIP", streamIPs[i]);

    grid->addWidget(wrapper, i / 3, i % 3);
  }

  scrollArea->setWidget(middleBox);
  layout->addWidget(scrollArea, 76);

  // Top Button Padding
  QWidget *topButtonBox = new QWidget();
  topButtonBox->setStyleSheet(R"(
  QWidget {
    border-top: 2px solid rgb(28, 34, 38);  /* Use your theme color */
    background-color: transparent; /* Optional: keep background clean */
  }
)");
  layout->addWidget(topButtonBox, 2);

  // Close Button
  QPushButton *closeButton = new QPushButton("Close");
  connect(closeButton, &QPushButton::clicked, this, &QWidget::close);
  QWidget *closeButtonWrapper = new QWidget();
  QHBoxLayout *closeButtonLayout = new QHBoxLayout(closeButtonWrapper);
  closeButtonLayout->setContentsMargins(1000, 0, 40, 0);
  closeButton->setStyleSheet(R"(
  QPushButton {
    background-color: rgb(54, 120, 156);
    border-radius: 18px;
  }
)");
  closeButtonLayout->addWidget(closeButton);
  layout->addWidget(closeButtonWrapper, 5);

  // Bottom Button Padding
  QWidget *bottomButtonBox = new QWidget();
  layout->addWidget(bottomButtonBox, 2);

  return page;
}

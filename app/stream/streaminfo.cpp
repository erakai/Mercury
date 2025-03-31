#include "streaminfo.hpp"

#include <QDateTime>
#include <QPropertyAnimation>
#include <qevent.h>

StreamInfo::StreamInfo(QWidget *parent, const QString &stream_title,
                       const QString &host_name)
{
  main_layout = new QHBoxLayout(this);
  basic_stream_info_layout = new QVBoxLayout();

  stream_title_label = new QLabel(stream_title, this);
  stream_title_label->setStyleSheet(
      "    color: #f0f0f0;"             // Almost white text
      "    font-size: 36px;"            // Large text
      "    font-weight: bold;"          // Bold text
      );

  // host_name_label = new QLabel(this);
  // host_name_label->setText("Host: {}" + host_name);
  host_name_label = new QLabel("Host: " + host_name, this);
  host_name_label->setStyleSheet(
      "    color: #f0f0f0;"             // Almost white text
      "    font-size: 18px;"            // Large text
      // "    font-weight: bold;"          // Bold text
  );

  // set viewer count
  viewer_count_label = new QLabel("Viewers: 1", this);
  viewer_count_label->setStyleSheet(
      "    color: #f0f0f0;"             // Almost white text
      "    font-size: 18px;"            // Large text
      // "    font-weight: ;"          // Bold text
  );


  basic_stream_info_layout->addWidget(stream_title_label);
  basic_stream_info_layout->addWidget(host_name_label);
  basic_stream_info_layout->addWidget(viewer_count_label);
  basic_stream_info_layout->addStretch();

  extra_info_sidebar = new QWidget(this);
  extra_info_sidebar->setStyleSheet("    background-color: #333333;"  // Dark gray background
      "    border-radius: 10px;"        // Rounded edges
      "    padding: 15px;"              // Inner spacing
  );
  extra_info_sidebar->setFixedSize(400, height()); // Adjust width if needed
  extra_info_sidebar->move(this->width(), 0); // Start off-screen
  main_layout->addLayout(basic_stream_info_layout);

  // adding info to sidebar
  extra_info_sidebar_layout = new QVBoxLayout(extra_info_sidebar);
  extra_info_sidebar_layout->setSpacing(2);

  stream_start_time_label = new QLabel("Stream Info:", extra_info_sidebar);
  stream_start_time_label->setStyleSheet("color: #f0f0f0; font-size: 16px; font-weight: bold; padding: 4px 8px");
  info_label2 = new QLabel("Viewers: 120", extra_info_sidebar);
  info_label2->setStyleSheet("color: #dddddd; font-size: 14px; padding: 4px 8px");
  info_label3 = new QLabel("Uptime: 2h 15m", extra_info_sidebar);
  info_label3->setStyleSheet("color: #dddddd; font-size: 14px; padding: 4px 8px");

  // Add labels to layout
  extra_info_sidebar_layout->addWidget(stream_start_time_label);
  extra_info_sidebar_layout->addWidget(info_label2);
  extra_info_sidebar_layout->addWidget(info_label3);
  extra_info_sidebar_layout->addWidget(info_label3);
  extra_info_sidebar_layout->addStretch();  // Push content to the top


  extra_info_sidebar_animation = new QPropertyAnimation(extra_info_sidebar, "geometry");
  extra_info_sidebar_animation->setDuration(300);

}


void StreamInfo::toggleExtraInfoSidebar() {
  int sidebarWidth = extra_info_sidebar->width();
  int peekWidth = 20;  // Keep 20px visible when closed

  QRect hiddenPos(width() - peekWidth, 0, sidebarWidth, extra_info_sidebar->height());
  QRect visiblePos(width() - sidebarWidth + peekWidth, 0, sidebarWidth, extra_info_sidebar->height());

  extra_info_sidebar_animation->stop();
  extra_info_sidebar_animation->setStartValue(isExtraInfoOpen ? visiblePos : hiddenPos);
  extra_info_sidebar_animation->setEndValue(isExtraInfoOpen ? hiddenPos : visiblePos);
  extra_info_sidebar_animation->start();

  isExtraInfoOpen = !isExtraInfoOpen;
}

void StreamInfo::resizeEvent(QResizeEvent *event) {
  QWidget::resizeEvent(event);  // Call the base class implementation

  // Keep sidebar height in sync with the parent
  extra_info_sidebar->setFixedSize(width() / 2, height());
  int peekWidth = 20;

  // Update the sidebar position based on its visibility state
  if (isExtraInfoOpen) {
    extra_info_sidebar->move(width() - extra_info_sidebar->width() + peekWidth, 0);
  } else {
    extra_info_sidebar->move(width() - peekWidth, 0); // Keep it off-screen
  }
}

void StreamInfo::mousePressEvent(QMouseEvent *event) {
  if (extra_info_sidebar->geometry().contains(event->pos())) {
    toggleExtraInfoSidebar();
  }
}

// void StreamInfo::setupLayout()
// {
//   info_layout->addWidget(host_name_label, 1, 0);
//   info_layout->addWidget(viewer_count_label, 1, 1);
// }

void StreamInfo::setViewerCount(int count)
{
  if (count < 0)
  {
    qDebug() << "StreamInfo::setViewerCount: negative count is not allowed";
    return;
  }
  viewer_count_label->setText(("Viewers: " + std::to_string(count)).c_str());
}

void StreamInfo::setStreamTitle(const QString &stream_title)
{
  stream_title_label->setText(stream_title);
}

void StreamInfo::setHostName(const QString &host_name)
{
  host_name_label->setText("Host: " + host_name);
}

void StreamInfo::setStreamStartTime(uint32_t timestamp)
{
  // timestamp is seconds since epoch
  stream_start_time = new QDateTime(QDateTime::fromSecsSinceEpoch(timestamp));
  QString formattedDt = stream_start_time->toString("yyyy-MM-dd HH:mm:ss");
  stream_start_time_label->setText(formattedDt);
}

std::string StreamInfo::getStreamTitle()
{
  return stream_title_label->text().toStdString();
}

uint32_t StreamInfo::getStreamStartTime()
{
  return stream_start_time->toSecsSinceEpoch();
}

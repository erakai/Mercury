#include "stream/streaminfo.hpp"
#include "home/toastnotification.h"
#include "stream/streamwindow.hpp"

#include <QDateTime>
#include <QPropertyAnimation>
#include <cstring>
#include <qevent.h>

StreamInfo::StreamInfo(QWidget *parent, const QString &stream_title,
                       const QString &host_name)
{
  StreamWindow *stream_window = qobject_cast<StreamWindow *>(parent);
  is_host = stream_window->is_host();
  main_layout = new QHBoxLayout;
  this->setLayout(main_layout);
  basic_stream_info_layout = new QVBoxLayout;

  stream_title_label = new QLabel(stream_title, this);
  // stream_title_label->setStyleSheet("    color: #f0f0f0;"  // Almost white
  // text
  //                                   "    font-size: 36px;" // Large text
  //                                   "    font-weight: bold;" // Bold text
  // );
  stream_title_label->setProperty("labelStyle", "title");

  host_name_label = new QLabel("Host: " + host_name, this);
  host_name_label->setProperty("labelStyle", "info");
  auto *host_name_container = new QWidget(this);
  auto *host_name_layout = new QHBoxLayout;
  host_name_container->setProperty("containerStyle", "info");
  host_name_container->setLayout(host_name_layout);
  host_name_layout->addWidget(host_name_label);
  host_name_container->setSizePolicy(QSizePolicy::Fixed,
                                     QSizePolicy::Preferred);
  // host_name_layout->addStretch();

  viewer_count_icon = new QLabel(this);
  QString app_dir = QCoreApplication::applicationDirPath();
  QString file(app_dir + "/assets/viewer-count-icon.png");
  QPixmap vc_pix(file);
  QIcon ico(vc_pix);
  viewer_count_icon->setPixmap(ico.pixmap({24, 24}));
  viewer_count_label = new QLabel("Viewers: 1", this);
  viewer_count_label->setProperty("labelStyle", "info");
  // viewer_count_label->setStyleSheet("background-color: rgb(255, 255, 255);");
  auto *viewer_count_container = new QWidget(this);
  // viewer_count_container->setStyleSheet(
  //     "border-radius: 10px; background-color: #333333;");
  auto *viewer_count_layout = new QHBoxLayout;
  viewer_count_container->setLayout(viewer_count_layout);
  viewer_count_container->setProperty("containerStyle", "info");
  viewer_count_layout->addWidget(viewer_count_icon);
  viewer_count_layout->addWidget(viewer_count_label);
  viewer_count_container->setSizePolicy(QSizePolicy::Fixed,
                                        QSizePolicy::Preferred);
  // viewer_count_layout->addStretch();

  basic_stream_info_layout->addWidget(stream_title_label);
  basic_stream_info_layout->addWidget(host_name_container);
  basic_stream_info_layout->addWidget(viewer_count_container);
  if (stream_window->is_client())
  {
    unstable_network_indicator = new QLabel(this);
    QString app_dir = QCoreApplication::applicationDirPath();
    QString filep(app_dir + "/assets/unstable-indicator.png");
    QPixmap pix(filep);
    QIcon ico(pix);
    unstable_network_indicator->setPixmap(ico.pixmap({48, 48}));
    unstable_network_indicator->setVisible(false);
    basic_stream_info_layout->addWidget(unstable_network_indicator);
  }
  basic_stream_info_layout->addStretch();

  extra_info_sidebar = new QWidget(this);
  extra_info_sidebar->setProperty("containerStyle", "info");
  // extra_info_sidebar->setStyleSheet(
  //     "background-color: #333333; border-radius: 10px; padding: 24px;");
  extra_info_sidebar->setFixedSize(400, height()); // Adjust width if needed
  extra_info_sidebar->move(this->width(), 0);      // Start off-screen
  auto *basic_stream_info_container = new QWidget(this);
  basic_stream_info_container->setLayout(basic_stream_info_layout);
  main_layout->addWidget(basic_stream_info_container);

  // reactions
  reaction_panel = new ReactionPanel(this);
  main_layout->addStretch();
  main_layout->addWidget(reaction_panel);
  main_layout->setContentsMargins(10, 10, 40, 10);
  connect(reaction_panel, &ReactionPanel::reactionClicked, this,
          &StreamInfo::handleReactionPanelButtonPressed);
  // reaction_panel->stackUnder(extra_info_sidebar);
  extra_info_sidebar->raise();

  // adding info to sidebar
  extra_info_sidebar_layout = new QVBoxLayout(extra_info_sidebar);
  extra_info_sidebar->setContentsMargins(8, 8, 8, 8);

  stream_start_time_label = new QLabel("Stream Start: ", extra_info_sidebar);
  // stream_start_time_label->setStyleSheet(
  //     "color: #dddddd; font-size: 18px; padding: 0px;");
  stream_start_time_label->setSizePolicy(QSizePolicy::Preferred,
                                         QSizePolicy::Fixed);

  stream_duration_label =
      new QLabel("Stream Duration: 00:00:00", extra_info_sidebar);
  // stream_duration_label->setStyleSheet(
  //     "color: #dddddd; font-size: 18px; padding: 0px;");
  stream_duration_label->setSizePolicy(QSizePolicy::Preferred,
                                       QSizePolicy::Fixed);
  seconds_timer = new QTimer(this);
  connect(seconds_timer, &QTimer::timeout, this,
          &StreamInfo::updateStreamDuration);
  seconds_timer->start(1000);

  extra_info_viewer_count_label =
      new QLabel("Viewer Count: 1", extra_info_sidebar);
  // extra_info_viewer_count_label->setStyleSheet(
  //     "color: #dddddd; font-size: 18px; padding: 0px;");
  extra_info_viewer_count_label->setSizePolicy(QSizePolicy::Preferred,
                                               QSizePolicy::Fixed);

  reactions_enabled_label = new QLabel("Reactions: On", extra_info_sidebar);
  // reactions_enabled_label->setStyleSheet(
  //     "color: #dddddd; font-size: 18px; padding: 0px;");
  reactions_enabled_label->setSizePolicy(QSizePolicy::Preferred,
                                         QSizePolicy::Fixed);

  // Add labels to layout
  extra_info_sidebar_layout->addWidget(stream_start_time_label);
  extra_info_sidebar_layout->addWidget(stream_duration_label);
  extra_info_sidebar_layout->addWidget(extra_info_viewer_count_label);
  extra_info_sidebar_layout->addWidget(reactions_enabled_label);
  extra_info_sidebar_layout->addStretch(); // Push content to the top

  extra_info_sidebar_animation =
      new QPropertyAnimation(extra_info_sidebar, "geometry");
  extra_info_sidebar_animation->setDuration(300);
}

void StreamInfo::initializeControlPanel()
{
  stream_control_panel = new StreamControlPanel(this);
  main_layout->insertWidget(2, stream_control_panel);
  stream_control_panel->stackUnder(extra_info_sidebar);

  // connect buttons
  stream_control_panel->setReactionsEnabledCheckBox(reactions_enabled);
  connect(stream_control_panel, &StreamControlPanel::reactionsEnabledChanged,
          this, &StreamInfo::handleReactionsEnabledChanged);
}

void StreamInfo::toggleExtraInfoSidebar()
{
  int sidebarWidth = extra_info_sidebar->width();
  int peekWidth = 20; // Keep 20px visible when closed

  QRect hiddenPos(width() - peekWidth, 0, sidebarWidth,
                  extra_info_sidebar->height());
  QRect visiblePos(width() - sidebarWidth + peekWidth, 0, sidebarWidth,
                   extra_info_sidebar->height());

  extra_info_sidebar_animation->stop();
  extra_info_sidebar_animation->setStartValue(isExtraInfoOpen ? visiblePos
                                                              : hiddenPos);
  extra_info_sidebar_animation->setEndValue(isExtraInfoOpen ? hiddenPos
                                                            : visiblePos);
  extra_info_sidebar_animation->start();

  isExtraInfoOpen = !isExtraInfoOpen;
}

void StreamInfo::resizeEvent(QResizeEvent *event)
{
  QWidget::resizeEvent(event); // Call the base class implementation

  // Keep sidebar height in sync with the parent
  extra_info_sidebar->setFixedSize(width() / 2, height());
  int peekWidth = 20;

  // Update the sidebar position based on its visibility state
  if (isExtraInfoOpen)
  {
    extra_info_sidebar->move(width() - extra_info_sidebar->width() + peekWidth,
                             0);
  }
  else
  {
    extra_info_sidebar->move(width() - peekWidth, 0); // Keep it off-screen
  }
}

void StreamInfo::mousePressEvent(QMouseEvent *event)
{
  if (extra_info_sidebar->geometry().contains(event->pos()))
  {
    toggleExtraInfoSidebar();
  }
}

void StreamInfo::setUnstableNetworkIndicator(bool visible, string toolTip)
{
  unstable_network_indicator->setVisible(visible);
  unstable_network_indicator->setToolTip(toolTip.c_str());
}

void StreamInfo::setViewerCount(int count)
{
  if (count < 0)
  {
    qDebug() << "StreamInfo::setViewerCount: negative count is not allowed";
    return;
  }
  viewer_count_label->setText(("Viewers: " + std::to_string(count)).c_str());
  extra_info_viewer_count_label->setText(
      ("Viewer Count: " + std::to_string(count)).c_str());
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
  QString formattedDt = stream_start_time->toString("h:mm AP, MMMM d, yyyy");
  stream_start_time_label->setText("Stream Start: " + formattedDt);
}

void StreamInfo::setReactionsEnabled(bool enabled)
{
  // enable / disable reaction panel
  reactions_enabled = enabled;
  setReactionsEnabledLabel(enabled);

  reaction_panel->setEnabled(enabled);

  // If this is a client, also hide it (after the first time)
  if (!is_host && initial_reaction_value_set)
  {
    ToastNotification::showToast(this->parentWidget(),
                                 enabled ? "Reactions Enabled"
                                         : "Reactions Disabled",
                                 1000, ToastType::NOTICE);
    reaction_panel->setVisible(enabled);
  }
  initial_reaction_value_set = true;
}

void StreamInfo::setReactionsEnabledLabel(bool enabled)
{
  reactions_enabled_label->setText(QString("Reactions: ") +
                                   (enabled ? "On" : "Off"));
}

void StreamInfo::updateStreamDuration()
{
  if (!stream_start_time)
    return;

  uint32_t duration_in_sec = QDateTime::currentSecsSinceEpoch() -
                             stream_start_time->toSecsSinceEpoch();
  uint32_t seconds = duration_in_sec % 60;
  uint32_t minutes = (duration_in_sec / 60) % 60;
  uint32_t hours = (duration_in_sec / 60 / 60);

  QString timeString = QString("%1:%2:%3")
                           .arg(hours, 2, 10, QChar('0'))
                           .arg(minutes, 2, 10, QChar('0'))
                           .arg(seconds, 2, 10, QChar('0'));
  stream_duration_label->setText("Stream Duration: " + timeString);
}

std::string StreamInfo::getStreamTitle()
{
  return stream_title_label->text().toStdString();
}

uint32_t StreamInfo::getStreamStartTime()
{
  return stream_start_time->toSecsSinceEpoch();
}

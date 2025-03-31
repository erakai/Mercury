#ifndef STREAMINFO_H
#define STREAMINFO_H
#include <QGridLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QWidget>

class StreamInfo : public QWidget
{
public:
  StreamInfo(QWidget *parent, const QString &stream_title,
             const QString &host_name);
  void resizeEvent(QResizeEvent *event);
  void setUnstableNetworkIndicator(bool visible, std::string toolTip);
  void mousePressEvent(QMouseEvent *event);
  void setViewerCount(int count);
  void setStreamTitle(const QString &stream_title);
  void setStreamStartTime(uint32_t timestamp);
  void setHostName(const QString &host_name);
  std::string getStreamTitle();
  uint32_t getStreamStartTime();

private:
  // void setupLayout();
  void toggleExtraInfoSidebar();
  bool isExtraInfoOpen;
  QHBoxLayout *main_layout;
  QVBoxLayout *basic_stream_info_layout;
  QLabel *stream_title_label;
  QLabel *viewer_count_label;
  QLabel *host_name_label;

  // Only relevant if this is a client - displays itself when poor connection
  QLabel *unstable_network_indicator;

  QWidget *extra_info_sidebar;
  QPropertyAnimation *extra_info_sidebar_animation;
  QVBoxLayout *extra_info_sidebar_layout;
  QDateTime *stream_start_time;
  QLabel *stream_start_time_label;
  QLabel *info_label2;
  QLabel *info_label3;
};

#endif // STREAMINFO_H

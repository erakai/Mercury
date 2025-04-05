#ifndef STREAMINFO_H
#define STREAMINFO_H

#include "reactionpanel.hpp"
#include "stream/mutestreambutton.hpp"
#include "stream/volumecontrolwidget.hpp"
#include "streamcontrolpanel.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QPropertyAnimation>
#include <QWidget>

class StreamInfo : public QWidget
{
  Q_OBJECT;

public:
  StreamInfo(QWidget *parent, const QString &stream_title,
             const QString &host_name);
  void resizeEvent(QResizeEvent *event);
  void setUnstableNetworkIndicator(bool visible, std::string toolTip);
  void mousePressEvent(QMouseEvent *event);
  void setViewerCount(int count);
  void setStreamTitle(const QString &stream_title);
  void setStreamStartTime(uint32_t timestamp);
  void setReactionsEnabled(bool enabled);
  void setHostName(const QString &host_name);
  std::string getStreamTitle();
  uint32_t getStreamStartTime();
  void initializeControlPanel();
  StreamControlPanel *getStreamControlPanel() { return stream_control_panel; };

signals:
  void renderAndSendReaction(ReactionPanel::Reaction reaction);
  void reactionsEnabledChanged(bool enabled);

private slots:
  void updateStreamDuration();
  void handleReactionPanelButtonPressed(ReactionPanel::Reaction reaction)
  {
    if (reactions_enabled)
    {
      emit renderAndSendReaction(reaction);
    }
  }
  void handleReactionsEnabledChanged(bool enabled)
  {
    // qDebug() << "reached stream info reactionsEnabledChanged --- " <<
    // enabled;
    emit reactionsEnabledChanged(enabled);
  }

private:
  // void setupLayout();
  void toggleExtraInfoSidebar();
  void setAnnotationsEnabledLabel(bool enabled);
  void setReactionsEnabledLabel(bool enabled);

  bool isExtraInfoOpen;
  QHBoxLayout *main_layout;
  QVBoxLayout *basic_stream_info_layout;
  VolumeControlWidget *volume_control;
  MuteStreamButton *mute_stream_btn;
  QLabel *stream_title_label;
  QLabel *viewer_count_icon;
  QLabel *viewer_count_label;
  QLabel *host_name_label;

  bool reactions_enabled;
  StreamControlPanel *stream_control_panel;
  ReactionPanel *reaction_panel;

  // Only relevant if this is a client - displays itself when poor connection
  QLabel *unstable_network_indicator;

  QWidget *extra_info_sidebar;
  QPropertyAnimation *extra_info_sidebar_animation;
  QVBoxLayout *extra_info_sidebar_layout;
  QDateTime *stream_start_time;
  QLabel *stream_start_time_label;
  QLabel *stream_duration_label;
  QTimer *seconds_timer;
  QLabel *extra_info_viewer_count_label;
  QLabel *reactions_enabled_label;
};

#endif // STREAMINFO_H

#pragma once

#include "annotationdisplay.hpp"
#include "sidepane.hpp"
#include "stream/painttoolwidget.hpp"
#include "stream/qualitycontrolbutton.hpp"
#include "streamdisplay.hpp"
#include "streamservice.hpp"
#include "hstp.hpp"
#include "reactiondisplay.hpp"
#include "streamdisplaycontrols.hpp"
#include "streaminfo.hpp"
#include <QAudioBuffer>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QMainWindow>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qwidget.h>

using namespace std;

enum MercuryMode
{
  HOST,
  CLIENT
};

class StreamWindow : public QMainWindow
{
  Q_OBJECT;

public:
  StreamWindow(string alias, shared_ptr<HostService> host_data,
               QWidget *parent = nullptr);
  StreamWindow(string alias, shared_ptr<ClientService> client_data,
               QWidget *parent = nullptr);

  bool set_up();

  bool is_host() { return mode == MercuryMode::HOST; }
  bool is_client() { return mode == MercuryMode::CLIENT; }

  bool provide_next_frame(QImage &next_video, QByteArray &next_audio);

  void set_has_host_muted_stream(bool is_muted)
  {
    has_host_muted_stream = is_muted;
  }

  void closeEvent(QCloseEvent *event) override;

public slots:
  // This should be connected to a signal from either MercuryClient (if this is
  // a client stream) that signals it has received enough information to fill
  // its jitter buffer, or to a signal from some window manager that signals it
  // has started to record the desktop. It won't play until this has been
  // called.
  void stream_fully_initialized();

  // Called to close the stream (if hosting) or leave (if client)
  void shut_down_window();

  // called when streamdisplaycontrols emits signal to change display mode
  void setStreamDisplayMode(int layout);

  // Sends a chat message out
  void send_chat_message(string message);

  // Sends a reaction out
  void send_reaction(ReactionPanel::Reaction reaction);

  // Sends an annotation out
  void send_annotation(HSTP_Annotation annotation);

  // These are generic update slots that should be connected to a signal either
  // in the client or server
  // The rest are generic update slots that should be connected to a signal
  // either in the client or server

  void viewer_count_updated(int new_count);
  void stream_name_changed(string host_alias, string new_name);
  void stream_start_time_changed(uint32_t timestamp);
  void reaction_permission_changed(uint32_t enabled);
  void new_chat_message(string alias, string msg);
  void new_reaction(string alias, uint32_t reaction);
  // void new_viewer_joined(Client client); // update participant_display_list
  // in side_pane
  void new_annotation(string alias, HSTP_Annotation annotation);

  void onAnnotationDisplayMousePressed(QMouseEvent *event);
  void onAnnotationDisplayMouseMoved(QMouseEvent *event);
  void onAnnotationDisplayMouseReleased(QMouseEvent *event);

  void onAnnotationCheckbox(int id, bool checked);
  void onAnnotationStatusChanged(bool checked);
  void onClearButtonClicked();

  void onClientSideQualityChanged(QualityOption quality);

  // These next two are host-only
  void viewer_disconnected(int id, std::string alias);
  // This is called after a client is validated
  void viewer_connected(int id, std::string _alias);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  /*
  Helper functions for setting the ui up.
  */
  void configure_menu_and_tool_bar();
  void connect_signals_and_slots();
  void initialize_primary_ui_widgets();

  // All important widgets we will be using to set up UI
  // Because a main window only has one central widget, we will be creating a
  // dummy one (display) to paste a layout onto.
  // https://doc.qt.io/qt-6/qtwidgets-tutorials-widgets-nestedlayouts-example.html.
  QWidget *display;
  QGridLayout *main_layout;
  SidePane *side_pane;
  StreamDisplay *stream_display;
  ReactionDisplay *reaction_display;
  AnnotationDisplay *annotation_display;
  QWidget *videoAnnotationContainer;
  StreamDisplayControls *stream_display_controls;
  PaintToolWidget *paint_tool;
  int streamDisplayMode = 0; // 0 def, 1 fullscreen, 2 tbh (no chat maybe)

  QualityOption client_side_stream_quality = QualityOption::DEFAULT;

  StreamInfo *stream_info;
  bool has_host_muted_stream = false;
  qint64 time_since_last_mutetoast = 0;
  const qint64 mutetoast_cooldown_ms = 10000;

  QMenu *stream_menu;
  QAction *stop_or_leave_stream_action;

  // Determines whether or not this window is a client or a host
  const MercuryMode mode;

  void updateEraseCursor(int radius);

  // Must be <18 characters
  string alias;

  // Only one of these will not be null - check mode first
  // These contain all specific data to being a host/client (networking, viewer
  // count, stream name, etc)
  const shared_ptr<HostService> servh = nullptr;
  const shared_ptr<ClientService> servc = nullptr;

  QPoint old_point;
  std::vector<QPoint> points;
};

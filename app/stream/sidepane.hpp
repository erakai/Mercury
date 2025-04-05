#pragma once

#include <QListWidget>
#include <QListWidgetItem>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QSettings>
#include <QTabWidget>
#include <QtCore/qtmetamacros.h>

#include "performancetab.hpp"

using namespace std;

struct ChatMessage
{
  string sender;
  string message;
};

class ChatTab : public QWidget
{
  Q_OBJECT;

public:
  ChatTab(const string &displayName, QWidget *parent = nullptr);

  void new_chat_message(ChatMessage msg, bool sender = false);
  void render_and_send_message(string msgContent);

signals:
  void send_chat_message(string message);

private:
  QListWidget *chatBox;
  QLineEdit *messageInput;
  string displayName;

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;
};

class ViewerListTab : public QWidget
{
  Q_OBJECT;

public:
  ViewerListTab(string host_alias, QWidget *parent = nullptr);

  void viewer_joined(int id, string alias);
  void viewer_left(int id, string alias);

signals:
  // New signal emitted when a viewer's checkbox is toggled.
  void viewer_checked(const int id, bool checked);

private:
  QListWidget *viewer_list;
  QLabel *label_list;
  vector<QListWidgetItem *> items;
};

class SidePane : public QTabWidget
{
  Q_OBJECT;

public:
  SidePane(QWidget *parent = nullptr);

  void initialize_chat_tab(const string &display_name);
  ChatTab *get_chat_tab() { return chat_tab; }

  void initialize_viewer_list_tab(const string &display_name);
  ViewerListTab *get_viewer_list_tab() { return viewer_list; }

  void initialize_server_performance_tab(shared_ptr<MercuryServer> server);
  ServerPerformanceTab *get_server_performance_tab()
  {
    return server_performance;
  }

  void initialize_client_performance_tab(shared_ptr<MercuryClient> client);
  ClientPerformanceTab *get_client_performance_tab()
  {
    return client_performance;
  }

  ChatTab *chat_tab;
  ViewerListTab *viewer_list;
  ServerPerformanceTab *server_performance;
  ClientPerformanceTab *client_performance;
};

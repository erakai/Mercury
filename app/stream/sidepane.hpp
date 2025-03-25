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
#include <QSettings>
#include <QTabWidget>
#include <QtCore/qtmetamacros.h>

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

  void new_chat_message(ChatMessage msg);
  void render_and_send_message(string msgContent);

signals:
  void send_chat_message(string message);

private:
  QListWidget *chatBox;
  QLineEdit *messageInput;
  string displayName;
};

class ViewerListTab : public QWidget
{
  Q_OBJECT;

public:
  ViewerListTab(string host_alias, QWidget *parent = nullptr);

  void viewer_joined(string alias);
  void viewer_left(string alias);

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

  ~SidePane()
  {
    if (chat_tab)
      delete chat_tab;
    if (viewer_list)
      delete viewer_list;
  }

  void initialize_chat_tab(const string &display_name);
  ChatTab *get_chat_tab() { return chat_tab; }

  void initialize_viewer_list_tab(const string &display_name);
  ViewerListTab *get_viewer_list_tab() { return viewer_list; }

private:
  ChatTab *chat_tab;
  ViewerListTab *viewer_list;
};

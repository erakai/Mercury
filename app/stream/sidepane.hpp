#pragma once

#include "../config/mconfig.hpp"

#include <QListWidget>
#include <QListWidgetItem>
#include <QPalette>
#include <QVBoxLayout>
#include <QWidget>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QSettings>

struct ChatMessage
{
  // maybe add timestamp?
  std::string sender;
  std::string message;
};

/*
Will have both chat and participants. I would check out:

https://doc.qt.io/qt-6/qtabwidget.html
*/
class SidePane : public QWidget
{
  Q_OBJECT;

public:
  SidePane(QWidget *parent = nullptr);

  void new_chat_message(ChatMessage msg);

  void render_and_send_message(std::string msgContent);

signals:
  void send_chat_message(std::string message);

private:
  QListWidget *chatBox;
  QLineEdit *messageInput;
};

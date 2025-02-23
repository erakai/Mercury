#pragma once

#include <QPalette>
#include <QWidget>

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
  SidePane(QWidget *parent) : QWidget(parent)
  {
    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::red);
    setAutoFillBackground(true);
    setPalette(pal);
  }

  void new_chat_message(ChatMessage msg);

private:
};

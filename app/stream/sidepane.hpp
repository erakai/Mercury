#pragma once

#include <QListWidget>
#include <QListWidgetItem>
#include <QPalette>
#include <QVBoxLayout>
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
    pal.setColor(QPalette::Window, Qt::gray);
    setAutoFillBackground(true);
    setPalette(pal);

    QVBoxLayout *layout = new QVBoxLayout(this);
    chatBox = new QListWidget(this);

    QPalette chatBoxPalette = chatBox->palette();
    chatBoxPalette.setColor(QPalette::Base, Qt::white);
    chatBoxPalette.setColor(QPalette::Text, Qt::black);
    chatBox->setPalette(chatBoxPalette);

    chatBox->setSelectionMode(QAbstractItemView::NoSelection);
    chatBox->setFocusPolicy(Qt::NoFocus);
    chatBox->setEditTriggers(QAbstractItemView::NoEditTriggers);
    chatBox->setEnabled(false);

    QString formattedMsg = QString::fromStdString("User 1:\nKai loves Alex\n");
    chatBox->addItem(new QListWidgetItem(formattedMsg));
    QString formattedMsg2 = QString::fromStdString("User 2:\nNo he doesn't\n");
    chatBox->addItem(new QListWidgetItem(formattedMsg2));

    layout->addWidget(chatBox);
    setLayout(layout);
  }

  void new_chat_message(ChatMessage msg);

signals:
  void send_chat_message(std::string message);

private:
  QListWidget *chatBox;
};

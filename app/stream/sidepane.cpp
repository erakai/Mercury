#include "sidepane.hpp"

SidePane::SidePane(QWidget *parent) : QWidget(parent)
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

void SidePane::new_chat_message(ChatMessage msg)
{
  // idk display it
  //TODO: render on front end
  //msg.sender = username
  //msg.message = message content
}

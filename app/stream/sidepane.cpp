#include "sidepane.hpp"

SidePane::SidePane(QWidget *parent, const std::string &displayName) : QWidget(parent), displayName(displayName)
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
  layout->addWidget(chatBox);

  QHBoxLayout *inputLayout = new QHBoxLayout();

  // QPushButton *sendButton = new QPushButton("Send", this);

  messageInput = new QLineEdit(this);
  QPalette inputPalette = messageInput->palette();
  inputPalette.setColor(QPalette::Base, Qt::white);
  inputPalette.setColor(QPalette::Text, Qt::black);
  messageInput->setPalette(inputPalette);

  inputLayout->addWidget(messageInput);
  // inputLayout->addWidget(sendButton);

  layout->addLayout(inputLayout);
  setLayout(layout);

  connect(messageInput, &QLineEdit::returnPressed, this,
          [this]()
          {
            render_and_send_message(
                messageInput->text().trimmed().toStdString());
          });
}

void SidePane::new_chat_message(ChatMessage msg)
{
  QString messageToRender =
      QString::fromStdString(msg.sender + ":\n" + msg.message + "\n");
  chatBox->addItem(new QListWidgetItem(messageToRender));
}

void SidePane::render_and_send_message(std::string msgContent)
{
  if (msgContent.length() == 0) {
    return;
  }

  messageInput->clear();

  new_chat_message({displayName, msgContent});

  emit send_chat_message(msgContent);
}

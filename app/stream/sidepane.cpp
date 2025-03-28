#include "sidepane.hpp"

SidePane::SidePane(QWidget *parent) : QTabWidget(parent)
{
  setTabPosition(QTabWidget::North);
}

void SidePane::initialize_chat_tab(const std::string &display_name)
{
  chat_tab = new ChatTab(display_name);
  addTab(chat_tab, "Chat");
}

void SidePane::initialize_viewer_list_tab(const string &display_name)
{
  viewer_list = new ViewerListTab(display_name);
  addTab(viewer_list, "Viewer List");
}

void SidePane::initialize_server_performance_tab(
    shared_ptr<MercuryServer> server)
{
  server_performance = new ServerPerformanceTab(server);
  addTab(server_performance, "Performance");
}

void SidePane::initialize_client_performance_tab(
    shared_ptr<MercuryClient> client)
{
  client_performance = new ClientPerformanceTab(client, client->get_alias());
  addTab(client_performance, "Performance");
}

ChatTab::ChatTab(const std::string &displayName, QWidget *parent)
    : QWidget(parent), displayName(displayName)
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

  messageInput = new QLineEdit(this);
  QPalette inputPalette = messageInput->palette();
  inputPalette.setColor(QPalette::Base, Qt::white);
  inputPalette.setColor(QPalette::Text, Qt::black);
  messageInput->setPalette(inputPalette);

  inputLayout->addWidget(messageInput);

  // QPushButton *sendButton = new QPushButton("Send", this);
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

void ChatTab::new_chat_message(ChatMessage msg)
{
  QString messageToRender =
      QString::fromStdString(msg.sender + ":\n" + msg.message + "\n");
  chatBox->addItem(new QListWidgetItem(messageToRender));
}

void ChatTab::render_and_send_message(std::string msgContent)
{
  if (msgContent.length() == 0)
    return;

  messageInput->clear();
  new_chat_message({displayName, msgContent});
  emit send_chat_message(msgContent);
}

ViewerListTab::ViewerListTab(string my_alias_str, QWidget *parent)
    : QWidget(nullptr)
{
  QBoxLayout *layout = new QVBoxLayout(this);
  layout->setSpacing(7);

  label_list = new QLabel("Viewers (0):");
  layout->addWidget(label_list);

  viewer_list = new QListWidget();
  layout->addWidget(viewer_list);

  QPalette list_palette = viewer_list->palette();
  list_palette.setColor(QPalette::Base, Qt::white);
  list_palette.setColor(QPalette::Text, Qt::black);
  viewer_list->setPalette(list_palette);
}

void ViewerListTab::viewer_joined(string alias)
{
  QListWidgetItem *new_item = new QListWidgetItem(alias.c_str());
  viewer_list->addItem(new_item);
  items.push_back(new_item);

  label_list->setText(QString("Viewers (%1):").arg(items.size()));
}

void ViewerListTab::viewer_left(string alias)
{
  for (int i = 0; i < items.size(); i++)
  {
    QListWidgetItem *item = items[i];
    if (item->text() == QString::fromStdString(alias))
    {
      int row = viewer_list->row(item);
      viewer_list->takeItem(row);
      items.erase(items.begin() + i);
      delete item;
      break;
    }
  }

  label_list->setText(QString("Viewers (%1):").arg(items.size()));
}

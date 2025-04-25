#include "sidepane.hpp"
#include <QtWidgets/qlistwidget.h>
#include <QMouseEvent>
#include <QApplication>

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
  setPalette(pal);

  QVBoxLayout *layout = new QVBoxLayout(this);
  chatBox = new QListWidget(this);

  chatBox->setSelectionMode(QAbstractItemView::NoSelection);
  chatBox->setFocusPolicy(Qt::NoFocus);
  chatBox->setEditTriggers(QAbstractItemView::NoEditTriggers);
  chatBox->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  chatBox->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  chatBox->setWordWrap(true);
  chatBox->setStyleSheet(R"(
  QListWidget {
    background-color: rgb(34, 34, 34);
    color: rgb(221, 231, 235);
    border: 1px solid;
    border-radius: 8px;
    padding: 4px;
  }
)");
  layout->addWidget(chatBox);

  QHBoxLayout *inputLayout = new QHBoxLayout();

  messageInput = new QLineEdit(this);
  messageInput->setPlaceholderText("Send a message");
  messageInput->setStyleSheet(R"(
  QLineEdit {
    border: 1px solid;
    border-radius: 5px;
    background-color: rgb(34, 34, 34);
    color: white;
    height: 30px;
    padding-left: 10px;
  }

  QLineEdit:focus {
    border: 1px solid rgb(54, 120, 156)
  }
)");

  inputLayout->addWidget(messageInput);
  layout->addLayout(inputLayout);
  setLayout(layout);

  connect(messageInput, &QLineEdit::returnPressed, this,
          [this]()
          {
            render_and_send_message(
                messageInput->text().trimmed().toStdString());
          });

  qApp->installEventFilter(this);
}

bool ChatTab::eventFilter(QObject *watched, QEvent *event)
{
  if (event->type() == QEvent::MouseButtonPress)
  {
    if (messageInput && messageInput->hasFocus())
    {
      QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
      QPoint globalPos = mouseEvent->globalPosition().toPoint();

      QWidget *clickedWidget = QApplication::widgetAt(globalPos);

      if (clickedWidget != messageInput)
      {
        messageInput->clearFocus();
      }
    }
  }

  return QWidget::eventFilter(watched, event);
}

void ChatTab::new_chat_message(ChatMessage msg, bool sender)
{
  QListWidgetItem *item = new QListWidgetItem();
  chatBox->addItem(item);

  QLabel *label = new QLabel();
  label->setTextFormat(Qt::RichText);

  QString senderRawText = QString::fromStdString(msg.sender);
  QString messageRawText = QString::fromStdString(msg.message);
  messageRawText.replace(QRegularExpression("([\\w\\d]{30})"), "\\1&#8203;");

  QString nameColor = "#4fc3f7";
  if (sender)
  {
    nameColor = "#d9a140";
  }

  QString richMessage = QString(R"(
  <span style="color:%1; font-weight:bold;">%2: </span><span style="color:#ffffff;">%3</span>
  )")
                            .arg(nameColor, senderRawText, messageRawText);

  label->setText(richMessage);
  label->setWordWrap(true);
  label->setStyleSheet(
      "background-color: transparent; padding: 0px 10px 0px 10px;");
  int maxWidth = chatBox->viewport()->width();
  label->setMaximumWidth(maxWidth);

  label->adjustSize();
  const int minHeight = 30;
  QSize labelSize = label->sizeHint();
  if (labelSize.height() < minHeight)
    labelSize.setHeight(minHeight);
  item->setSizeHint(labelSize);

  chatBox->setItemWidget(item, label);
  chatBox->scrollToBottom();
}

void ChatTab::render_and_send_message(std::string msgContent)
{
  if (msgContent.length() == 0)
    return;

  messageInput->clear();
  new_chat_message({displayName, msgContent}, true);
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
  viewer_list->setSelectionMode(QAbstractItemView::NoSelection);
  viewer_list->setFocusPolicy(Qt::NoFocus);
  viewer_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
  layout->addWidget(viewer_list);

  QPalette list_palette = viewer_list->palette();
  list_palette.setColor(QPalette::Base, QColor(64, 68, 69));
  list_palette.setColor(QPalette::Text, QColor(221, 231, 235));
  viewer_list->setPalette(list_palette);
}

void ViewerListTab::viewer_joined(int id, string alias)
{
  // Create a new list item and store the alias as its text.
  QListWidgetItem *new_item = new QListWidgetItem();
  new_item->setText(QString::fromStdString(alias));

  // Create a custom widget to hold the label and the checkbox.
  QWidget *item_widget = new QWidget();
  QHBoxLayout *layout = new QHBoxLayout(item_widget);
  layout->setContentsMargins(0, 0, 0, 0); // Remove margins for a cleaner look

  // Create the label displaying the alias.
  QLabel *label = new QLabel(QString::fromStdString(alias));
  // Create the checkbox that will appear to the right of the label.
  QCheckBox *checkBox = new QCheckBox();
  checkBox->setChecked(true);

  // Add the label, a stretch (to push the checkbox to the right), and the
  // checkbox to the layout.
  layout->addWidget(label);
  layout->addStretch();
  layout->addWidget(checkBox);

  // Add the new item to the list and assign the custom widget to it.
  viewer_list->addItem(new_item);
  viewer_list->setItemWidget(new_item, item_widget);
  items.push_back(new_item);

  // Connect the checkbox toggled signal to emit our custom signal with the
  // viewer's alias.
  connect(checkBox, &QCheckBox::toggled, this,
          [this, id](bool checked) { emit viewer_checked(id, checked); });

  // Update the viewers label.
  label_list->setText(QString("Viewers (%1):").arg(items.size()));
}

void ViewerListTab::viewer_left(int id, string alias)
{
  for (int i = 0; i < (int) items.size(); i++)
  {
    QListWidgetItem *item = items[i];
    // Identify the item by its stored text.
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

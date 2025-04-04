#include "streamcontrolpanel.hpp"

#include <QCheckBox>
#include <QVBoxLayout>

StreamControlPanel::StreamControlPanel(QWidget *parent)
{
  // StreamWindow *stream_window = qobject_cast<StreamWindow
  // *>(parent->parent());

  auto *panel_container = new QWidget(this);
  panel_container->setStyleSheet(
      "QWidget {"
      "    background-color: #222222;" // Dark background
      "    border-radius: 12px;"       // Rounded corners
      "    padding: 12px;"             // Internal spacing
      "}");

  // create control button layout
  auto *control_button_grid = new QVBoxLayout();
  control_button_grid->setSpacing(10);
  panel_container->setLayout(control_button_grid);

  // create reaction control checkbox
  reactions_enabled_check_box = new QCheckBox(parent);
  reactions_enabled_check_box->setText("Reactions Enabled");
  reactions_enabled_check_box->setStyleSheet(checkboxStyle);
  control_button_grid->addWidget(reactions_enabled_check_box);

  // connect reaction control to send signal
  connect(reactions_enabled_check_box, &QCheckBox::checkStateChanged, this,
          [this]
          {
            bool enabled = reactions_enabled_check_box->checkState() == 2;
            // qDebug() << "received reaction control in stream control panel
            // ---- "
            //          << enabled;
            emit reactionsEnabledChanged(enabled);
          });

  // connect(reaction_button, &QPushButton::clicked, this,
  //             [this, reaction]()
  //             {
  //               qDebug() << "reaction clicked, emitting reaction";
  //               emit reactionClicked(reaction);
  //             });

  if (true)
  {
    mute_stream_btn = new MuteStreamButton(this);
    control_button_grid->addWidget(mute_stream_btn);

    connect(mute_stream_btn, &MuteStreamButton::mute_status_changed, this,
            [this](bool is_muted) { emit mute_status_changed(is_muted); });
  }

  // create main layout in case we want to add other things
  auto *main_layout = new QVBoxLayout(this);
  control_button_grid->setContentsMargins(12, 12, 20, 12);
  main_layout->addWidget(panel_container);
  main_layout->addStretch();
  this->setLayout(main_layout);
}

void StreamControlPanel::setReactionsEnabledCheckBox(bool enabled)
{
  reactions_enabled_check_box->setChecked(enabled);
}

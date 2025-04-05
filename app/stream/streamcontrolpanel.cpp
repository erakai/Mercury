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
      "    border-radius: 8;"          // Rounded corners
      "    padding: 8;"                // Internal spacing
      "}");

  // create control button layout
  auto *control_button_grid = new QVBoxLayout();
  control_button_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
  panel_container->setLayout(control_button_grid);

  // create reaction control checkbox
  reactions_enabled_check_box = new QCheckBox(parent);
  reactions_enabled_check_box->setSizePolicy(QSizePolicy::Minimum,
                                             QSizePolicy::Preferred);
  reactions_enabled_check_box->setText("Reactions Enabled");
  reactions_enabled_check_box->setStyleSheet(checkboxStyle);
  control_button_grid->addWidget(reactions_enabled_check_box, 0, Qt::AlignLeft);

  // connect reaction control to send signal
  connect(reactions_enabled_check_box, &QCheckBox::checkStateChanged, this,
          [this]
          {
            bool enabled = reactions_enabled_check_box->checkState() == 2;
            emit reactionsEnabledChanged(enabled);
          });

  mute_stream_btn = new MuteStreamButton(this);
  mute_stream_btn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  mute_stream_btn->setStyleSheet(checkboxStyle);
  control_button_grid->addWidget(mute_stream_btn, 0, Qt::AlignLeft);

  connect(mute_stream_btn, &MuteStreamButton::mute_status_changed, this,
          [this](bool is_muted) { emit mute_status_changed(is_muted); });

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

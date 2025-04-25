#ifndef STREAMCONTROLPANEL_H
#define STREAMCONTROLPANEL_H

#include "stream/mutestreambutton.hpp"
#include <QWidget>

class QCheckBox;
class StreamControlPanel : public QWidget
{
  Q_OBJECT;

public:
  StreamControlPanel(QWidget *parent);
  void setReactionsEnabledCheckBox(bool enabled);

signals:
  void reactionsEnabledChanged(bool enabled);

  void mute_status_changed(bool is_muted);

private:
  QCheckBox *reactions_enabled_check_box;
  const QString checkboxStyle = R"(
    QCheckBox {
        color: #f0f0f0;
        font-size: 16px;
        spacing: 8px;
    }
    QCheckBox::indicator {
        width: 18px;
        height: 18px;
        border-radius: 4px;
        background: #222; /* Dark background */
        border: 2px solid #444; /* Subtle border */
    }
    QCheckBox::indicator:hover {
        border: 2px solid #666; /* Highlight on hover */
    }
    QCheckBox::indicator:checked {
        background: #087AA0; /* Green when checked */
        border: 2px solid #087AA0;
    }
    QCheckBox::indicator:checked::before {
        font-size: 14px;
        color: #f0f0f0;
        font-weight: bold;
        text-align: center;
        position: absolute;
        left: 4px;
        top: 2px;
    }
  )";

  MuteStreamButton *mute_stream_btn;
};

#endif // STREAMCONTROLPANEL_H

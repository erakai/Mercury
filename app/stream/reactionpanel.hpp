#ifndef REACTIONPANEL_H
#define REACTIONPANEL_H
#include <QWidget>

/*
 * Widget that displays reactions which can be pressed on to send to the stream
 */

class ReactionPanel : public QWidget
{
  Q_OBJECT
public:
  enum class Reaction
  {
    ThumbsUp,
    Heart,
    Laugh,
    Surprised,
    Fire,
    Sob,
    Angry,
    ThumbsDown,
    Skull
  };

  ReactionPanel(QWidget *parent);

  static QString getReactionAssetName(Reaction reaction);

signals:
  void reactionClicked(Reaction reaction);

private:
  std::vector<Reaction> reactionList = {
      Reaction::ThumbsUp,  Reaction::Heart,      Reaction::Laugh,
      Reaction::Surprised, Reaction::Fire,       Reaction::Sob,
      Reaction::Angry,     Reaction::ThumbsDown, Reaction::Skull};

  static const std::vector<std::string> reactionStringList;
};

#endif // REACTIONPANEL_H

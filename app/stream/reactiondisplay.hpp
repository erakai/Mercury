//
// Created by Christopher Lee on 4/2/25.
//

#ifndef REACTIONDISPLAY_H
#define REACTIONDISPLAY_H
#include "reactionpanel.hpp"

class ReactionDisplay : public QWidget {
public:
  explicit ReactionDisplay(QWidget* parent = nullptr);
  void addReaction(ReactionPanel::Reaction reaction);
signals:

private:
  void displayReaction(ReactionPanel::Reaction reaction);

};



#endif //REACTIONDISPLAY_H

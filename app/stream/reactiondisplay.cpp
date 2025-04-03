#include "reactiondisplay.hpp"

#include <QLabel>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QGraphicsOpacityEffect>

ReactionDisplay::ReactionDisplay(QWidget *parent)
{
  // setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
  setAttribute(Qt::WA_TranslucentBackground);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // setAttribute(Qt::WA_NoSystemBackground);
  // setAttribute(Qt::WA_AlwaysStackOnTop);
}

void ReactionDisplay::addReaction(ReactionPanel::Reaction reaction)
{
  displayReaction(reaction);
}

void ReactionDisplay::displayReaction(ReactionPanel::Reaction reaction)
{
  QString filepath = QString("assets/reactions/%1")
                         .arg(ReactionPanel::getReactionAssetName(reaction));
  QPixmap reactionPixmap(filepath);

  if (reactionPixmap.isNull())
  {
    qWarning() << "Failed to load reaction asset:" << filepath;
    return;
  }

  auto *reactionLabel = new QLabel(this);
  reactionLabel->setPixmap(reactionPixmap);
  reactionLabel->setAttribute(Qt::WA_TranslucentBackground);
  reactionLabel->setScaledContents(true);
  reactionLabel->setFixedSize(100, 100); // Adjust size if necessary

  // Random position within overlay bounds
  int x = QRandomGenerator::global()->bounded(width() - reactionLabel->width());
  int y =
      QRandomGenerator::global()->bounded(height() - reactionLabel->height());
  reactionLabel->move(x, y);
  reactionLabel->show();

  // Create fade-out effect
  auto *effect = new QGraphicsOpacityEffect(reactionLabel);
  reactionLabel->setGraphicsEffect(effect);

  auto *fadeOut = new QPropertyAnimation(effect, "opacity");
  fadeOut->setDuration(2000); // 2 seconds fade-out
  fadeOut->setStartValue(1.0);
  fadeOut->setEndValue(0.0);

  // Delete reaction when animation is complete
  connect(fadeOut, &QPropertyAnimation::finished,
          [reactionLabel, fadeOut, effect]()
          {
            delete fadeOut;               // Delete animation
            delete effect;                // Delete opacity effect
            reactionLabel->deleteLater(); // Delete QLabel safely
          });

  fadeOut->start();
}
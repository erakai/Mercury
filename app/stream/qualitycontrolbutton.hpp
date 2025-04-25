#pragma once

#include <QWidget>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QVBoxLayout>

enum class QualityOption
{
  DEFAULT = 0,
  Q240 = 1,
  Q480 = 2,
  Q720 = 3,
  Q1080 = 4,
};

class QualityControlButton : public QPushButton
{
  Q_OBJECT;

public:
  QualityControlButton(QWidget *parent = nullptr);

private:
  QAction *option0;
  QAction *option1;
  QAction *option2;
  QAction *option3;
  QAction *option4;

  void uncheck_all();

signals:
  void quality_changed(QualityOption new_quality);
};

#ifndef AUDIOINPUTLISTMODEL_H
#define AUDIOINPUTLISTMODEL_H

#include <QAbstractListModel>
#include <QMediaDevices>
#include <QAudioDevice>

QT_BEGIN_NAMESPACE
QT_END_NAMESPACE

class AudioInputListModel : public QAbstractListModel
{
  Q_OBJECT

public:
  explicit AudioInputListModel(QObject *parent = nullptr);

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QAudioDevice audioDevice(const QModelIndex &index) const;

private Q_SLOTS:
  void audioInputsChanged();
};

#endif // AUDIOINPUTLISTMODEL_H

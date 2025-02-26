// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "audioinputlistmodel.h"
#include <QMediaDevices>

AudioInputListModel::AudioInputListModel(QObject *parent)
    : QAbstractListModel(parent)
{

  auto *media = new QMediaDevices();
  // Connect to changes in the available audio inputs.
  connect(media, &QMediaDevices::audioInputsChanged, this,
          &AudioInputListModel::audioInputsChanged);
}

int AudioInputListModel::rowCount(const QModelIndex &) const
{
  return QMediaDevices::audioInputs().size();
}

QVariant AudioInputListModel::data(const QModelIndex &index, int role) const
{
  const auto inputs = QMediaDevices::audioInputs();
  if (!index.isValid() || index.row() < 0 || index.row() >= inputs.size())
    return {};

  if (role == Qt::DisplayRole)
    return inputs.at(index.row()).description();

  return {};
}

QAudioDevice AudioInputListModel::audioDevice(const QModelIndex &index) const
{
  const auto inputs = QMediaDevices::audioInputs();
  if (index.isValid() && index.row() >= 0 && index.row() < inputs.size())
    return inputs.at(index.row());
  return QAudioDevice();
}

void AudioInputListModel::audioInputsChanged()
{
  beginResetModel();
  endResetModel();
}

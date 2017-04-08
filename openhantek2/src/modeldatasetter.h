#ifndef MODELDATASETTER_H
#define MODELDATASETTER_H

#include <QAbstractItemModel>
#include <QObject>

class ModelDataSetter : public QObject
{
    Q_OBJECT
public:
  Q_INVOKABLE void setModelData(QAbstractItemModel *model,
                        int row,
                        const QString& role,
                        QVariant data);
//    Q_INVOKABLE void setModelData(QAbstractItemModel *model,
//                          int row,
//                          const QString& role,
//                          QColor &data);
};

#endif // MODELDATASETTER_H

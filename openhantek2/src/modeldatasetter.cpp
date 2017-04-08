#include "modeldatasetter.h"
#include <QColor>

void ModelDataSetter::setModelData(QAbstractItemModel *model,
                      int row,
                      const QString& role,
                      QVariant data)
{
  model->setData(model->index(row, 0), data, model->roleNames().key(role.toLatin1()));
}

//void ModelDataSetter::setModelData(QAbstractItemModel *model,
//                      int row,
//                      const QString& role,
//                      QColor &data)
//{
//  model->setData(model->index(row, 0), QVariant::fromValue(data), model->roleNames().key(role.toAscii()));
//}

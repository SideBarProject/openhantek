#ifndef QPFIXEDSCALEENGINE_H
#define QPFIXEDSCALEENGINE_H

#include "qpscaleengine.h"

class QPFixedScaleEngine : public QPScaleEngine
{
    Q_OBJECT
public:
    QPFixedScaleEngine(QQuickItem *parent = 0);

public slots:
    void updateScale();

    void setMin(qreal min);
    void setMax(qreal max);

protected:
    void calculateMinMax(qreal &min, qreal &max);
};

#endif // QPFIXEDSCALEENGINE_H

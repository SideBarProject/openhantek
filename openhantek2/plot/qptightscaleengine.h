#ifndef QPTIGHTSCALEENGINE_H
#define QPTIGHTSCALEENGINE_H

#include "qpscaleengine.h"

class QPTightScaleEngine : public QPScaleEngine
{
    Q_OBJECT
public:
    explicit QPTightScaleEngine(QQuickItem *parent = 0);

protected:
    void calculateMinMax(qreal &min, qreal &max);

};

#endif // QPTIGHTSCALEENGINE_H

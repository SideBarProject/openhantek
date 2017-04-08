#include "qpfixedscaleengine.h"

QPFixedScaleEngine::QPFixedScaleEngine(QQuickItem *parent) :
    QPScaleEngine(parent)
{
}

void QPFixedScaleEngine::updateScale()
{
    // nothing to do here - the scale is fixed
}

void QPFixedScaleEngine::setMin(qreal min)
{
    if (m_min == min)
        return;

    m_min = min;
    emit minChanged(min);
}

void QPFixedScaleEngine::setMax(qreal max)
{
    if (m_max == max)
        return;

    m_max = max;
    emit maxChanged(max);
}

void QPFixedScaleEngine::calculateMinMax(qreal &min, qreal &max)
{
    min = m_min;
    max = m_max;
}

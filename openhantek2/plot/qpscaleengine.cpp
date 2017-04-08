#include "qpplotitem.h"
#include "qpscaleengine.h"

QPScaleEngine::QPScaleEngine(QQuickItem *parent) :
    QQuickItem(parent), m_min(0), m_max(0), m_axis(Qt::YAxis)
{
}

qreal QPScaleEngine::min() const
{
    return m_min;
}

qreal QPScaleEngine::max() const
{
    return m_max;
}

QList<QPPlotItem *> QPScaleEngine::plotItems() const
{
    return m_plotItems;
}

void QPScaleEngine::addPlotItem(QPPlotItem *item)
{
    if (!m_plotItems.contains(item)) {
        m_plotItems.append(item);
        connect(item, &QPPlotItem::coordinateBoundsChanged, this, &QPScaleEngine::updateScale);
        updateScale();
    }
}

void QPScaleEngine::removePlotItem(QPPlotItem *item)
{
    disconnect(item, &QPPlotItem::coordinateBoundsChanged, this, &QPScaleEngine::updateScale);
    m_plotItems.removeOne(item);
    updateScale();
}

void QPScaleEngine::clearPlotItems()
{
    foreach (QPPlotItem *item, m_plotItems) {
        disconnect(item, &QPPlotItem::coordinateBoundsChanged, this, &QPScaleEngine::updateScale);
    }
    m_plotItems.clear();
}

Qt::Axis QPScaleEngine::axis() const
{
    return m_axis;
}

qreal QPScaleEngine::map(qreal val, const QRectF &rect)
{
    if (m_axis == Qt::YAxis) {
        return (min() - val) / (max() - min()) * rect.height() + rect.height();
    } else if (m_axis == Qt::XAxis) {
        return (val - min()) / (max() - min()) * rect.width();
    }

    return 0;
}

qreal QPScaleEngine::map(qreal val, QQuickItem *item)
{
    return map(val, item->boundingRect());
}

qreal QPScaleEngine::scaleFactor(const QRectF &rect)
{
    if (m_axis == Qt::YAxis) {
        return -rect.height() / (max() - min());
    } else if (m_axis == Qt::XAxis) {
        return rect.width() / (max() - min());
    }

    return 0;
}

qreal QPScaleEngine::scaleFactor(QQuickItem *item)
{
    return scaleFactor(item->boundingRect());
}

void QPScaleEngine::updateScale()
{
    qreal min = 0, max = 0;
    calculateMinMax(min, max);
    if (min != m_min) {
        m_min = min;
        emit minChanged(m_min);
    }
    if (max != m_max) {
        m_max = max;
        emit maxChanged(m_max);
    }
}

void QPScaleEngine::setAxis(Qt::Axis arg)
{
    if (m_axis == arg)
        return;

    m_axis = arg;
    emit axisChanged(arg);
}

void QPScaleEngine::setMin(qreal min)
{
    Q_UNUSED(min)
    qWarning("This scale engine dynamically calculates its bounds. Setting the minimum will not have an effect.");
}

void QPScaleEngine::setMax(qreal max)
{
    Q_UNUSED(max)
    qWarning("This scale engine dynamically calculates its bounds. Setting the maximum will not have an effect.");
}

void QPScaleEngine::calculateMinMax(qreal &min, qreal &max)
{
    Q_UNUSED(min);
    Q_UNUSED(max);
}

static void plotitemlist_append(QQmlListProperty<QPPlotItem> *p, QPPlotItem *v) {
    static_cast<QPScaleEngine*>(p->object)->addPlotItem(v);
}
static int plotitemlist_count(QQmlListProperty<QPPlotItem> *p) {
    return reinterpret_cast<QList<QPPlotItem*>*>(p->data)->count();
}
static QPPlotItem *plotitemlist_at(QQmlListProperty<QPPlotItem> *p, int idx) {
    return reinterpret_cast<QList<QPPlotItem*>*>(p->data)->at(idx);
}
static void plotitemlist_clear(QQmlListProperty<QPPlotItem> *p) {
    static_cast<QPScaleEngine*>(p->object)->clearPlotItems();
}

QQmlListProperty<QPPlotItem> QPScaleEngine::plotItemList() {
    return QQmlListProperty<QPPlotItem>(this, &m_plotItems, plotitemlist_append, plotitemlist_count, plotitemlist_at, plotitemlist_clear);
}

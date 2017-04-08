#ifndef QPSCALEENGINE_H
#define QPSCALEENGINE_H

#include <QQuickItem>

class QPPlotItem;

class QPScaleEngine : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(qreal min READ min WRITE setMin NOTIFY minChanged)
    Q_PROPERTY(qreal max READ max WRITE setMax NOTIFY maxChanged)
    Q_PROPERTY(QQmlListProperty<QPPlotItem> plotItems READ plotItemList)
    Q_PROPERTY(Qt::Axis axis READ axis WRITE setAxis NOTIFY axisChanged)

public:
    explicit QPScaleEngine(QQuickItem *parent = 0);

    qreal min() const;
    qreal max() const;

    QList<QPPlotItem*> plotItems() const;

    Qt::Axis axis() const;

    Q_INVOKABLE qreal map(qreal val, const QRectF &rect);
    Q_INVOKABLE qreal map(qreal val, QQuickItem *item);

    Q_INVOKABLE qreal scaleFactor(const QRectF &rect);
    Q_INVOKABLE qreal scaleFactor(QQuickItem *item);

signals:
    void minChanged(qreal arg);
    void maxChanged(qreal arg);
    void axisChanged(Qt::Axis arg);

public slots:

    void addPlotItem(QPPlotItem *item);
    void removePlotItem(QPPlotItem *item);
    void clearPlotItems();

    virtual void updateScale();
    void setAxis(Qt::Axis arg);

    virtual void setMin(qreal min);
    virtual void setMax(qreal max);

protected:
    virtual void calculateMinMax(qreal &min, qreal &max);

    QQmlListProperty<QPPlotItem> plotItemList();

    qreal m_min, m_max;
    QList<QPPlotItem*> m_plotItems;
    Qt::Axis m_axis;
};

#endif // QPSCALEENGINE_H

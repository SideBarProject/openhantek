#ifndef QPPLOTITEM_H
#define QPPLOTITEM_H

#include <QQuickItem>

class QPScaleEngine;

class QPPlotItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QRectF coordinateBounds READ coordinateBounds NOTIFY coordinateBoundsChanged)

public:
    explicit QPPlotItem(QQuickItem *parent = 0);
    ~QPPlotItem();

    virtual QRectF coordinateBounds() const;

signals:
    void coordinateBoundsChanged(const QRectF &arg);
};

#endif // QPPLOTITEM_H

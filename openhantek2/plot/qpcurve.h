#ifndef QPCURVE_H
#define QPCURVE_H

#include <QVector>
#include "qpplotitem.h"

class QPCurve : public QPPlotItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<QPointF> data READ data WRITE setData)
    Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
public:
    explicit QPCurve(QQuickItem *parent = 0);

    QVector<QPointF> data() const;

    QRectF coordinateBounds() const;

    QColor color() const;

public slots:
    void clear();
    void setData(const QVector<QPointF> &data);
    void setData(const QVector<float> &data);

    void appendData(const QVector<QPointF> &data);
    void appendData(const QVector<float> &data);

    void setColor(const QColor &arg);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

signals:
    void colorChanged(const QColor &arg);

private:
    QVector<QPointF> m_data;
    QRectF m_bounds;
    QColor m_color;
};

#endif // QPCURVE_H

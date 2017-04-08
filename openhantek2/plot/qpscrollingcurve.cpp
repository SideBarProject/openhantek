#include "qpscrollingcurve.h"

#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QSGFlatColorMaterial>

QPScrollingCurve::QPScrollingCurve(QQuickItem *parent) :
    QPPlotItem(parent), m_numPoints(100), m_data(100), m_color(Qt::blue)
{
}

int QPScrollingCurve::numPoints() const
{
    return m_numPoints;
}

void QPScrollingCurve::setNumPoints(int arg)
{
    if (arg <= 0)
        return;

    if (m_numPoints != arg) {
        m_numPoints = arg;
        if (static_cast<uint>(m_numPoints) < m_data.size()) {
            const size_t diff = m_data.size() - m_numPoints;
            auto last = m_data.begin() + diff;
            for (auto it = m_data.begin(); it != last; ++it) {
                m_orderedData.erase(m_orderedData.find(*it));
            }
        }
        m_data.rset_capacity(m_numPoints);
        emit numPointsChanged(arg);
        emit coordinateBoundsChanged(coordinateBounds());
        update();
    }
}

QColor QPScrollingCurve::color() const
{
    return m_color;
}

QRectF QPScrollingCurve::coordinateBounds() const
{
    QRectF bounds;
    bounds.setCoords(0, yMax(), numPoints(), yMin());
    return bounds;
}

void QPScrollingCurve::appendDataPoints(const QVector<float> &data)
{
    if (data.isEmpty())
        return;

    float oldMin = yMin();
    float oldMax = yMax();

    for (float f : data) {
        if (m_data.full()) {
            m_orderedData.erase(m_orderedData.find(m_data.front()));
        }
        m_data.push_back(f);
        m_orderedData.insert(f);
    }

    if (yMin() != oldMin || yMax() != oldMax) {
        emit coordinateBoundsChanged(coordinateBounds());
    }

    update();
}

void QPScrollingCurve::appendDataPoint(qreal v)
{
    appendDataPoints(QVector<float>() << v);
}

void QPScrollingCurve::setColor(const QColor &arg)
{
    if (m_color != arg) {
        m_color = arg;
        emit colorChanged(arg);
        update();
    }
}

QSGNode *QPScrollingCurve::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    QSGGeometryNode *node = 0;
    QSGGeometry *geometry = 0;
    QSGFlatColorMaterial *material = 0;

    if (!oldNode) {
        node = new QSGGeometryNode;
        geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), m_data.size());
        geometry->setLineWidth(2);
        geometry->setDrawingMode(GL_LINE_STRIP);
        node->setGeometry(geometry);
        node->setFlag(QSGNode::OwnsGeometry);
        material = new QSGFlatColorMaterial;
        material->setColor(m_color);
        node->setMaterial(material);
        node->setFlag(QSGNode::OwnsMaterial);
        node->markDirty(QSGNode::DirtyMaterial);
    } else {
        node = static_cast<QSGGeometryNode *>(oldNode);
        geometry = node->geometry();
        geometry->allocate(m_data.size());
        material = static_cast<QSGFlatColorMaterial*>(node->material());
        if (material->color() != m_color) {
            material->setColor(m_color);
            node->markDirty(QSGNode::DirtyMaterial);
        }
    }

    QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();

    for (uint i = 0; i < m_data.size(); ++i) {
        QPointF p(i, m_data[i]);
        vertices[i].set(p.x(), p.y());
    }

    node->markDirty(QSGNode::DirtyGeometry);

    return node;
}

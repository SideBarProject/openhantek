#include "scopecolors.h"

#include <QSettings>
#include <QDebug>

ScopeColors::ScopeColors(const QString& type) : m_type(type)
{
    readColors();
}

QColor ScopeColors::getAxes() const
{
    return axes;
}

void ScopeColors::setAxes(const QColor& value)
{
    axes = value;
}

QColor ScopeColors::getBackground() const
{
    return background;
}

void ScopeColors::setBackground(const QColor& value)
{
    background = value;
}

QColor ScopeColors::getBorder() const
{
    return border;
}

void ScopeColors::setBorder(const QColor& value)
{
    border = value;
}

QColor ScopeColors::getGrid() const
{
    return grid;
}

void ScopeColors::setGrid(const QColor& value)
{
    grid = value;
}

QColor ScopeColors::getMarkers() const
{
    return markers;
}

void ScopeColors::setMarkers(const QColor& value)
{
    markers = value;
}

QColor ScopeColors::getText() const
{
    return text;
}

void ScopeColors::setText(const QColor& value)
{
    text = value;
}

QString ScopeColors::type() const
{
    return m_type;
}

void ScopeColors::setType(const QString& type)
{
    m_type = type;
    readColors();
}

int ScopeColors::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 6 + voltage.size() + spectrum.size();
}

int ScopeColors::columnCount(const QModelIndex& parent) const
{
    return 2;
}

QVariant ScopeColors::data(const QModelIndex& index, int role) const
{
    switch(index.row()) {
        case 0: if (role==NameRole) return tr("Axes"); else return axes;
        case 1: if (role==NameRole) return tr("Background"); else return background;
        case 2: if (role==NameRole) return tr("Border"); else return border;
        case 3: if (role==NameRole) return tr("Grid"); else return grid;
        case 4: if (role==NameRole) return tr("Markers"); else return markers;
        case 5: if (role==NameRole) return tr("Text"); else return text;
    default:
        if (index.row()<6+(int)spectrum.size()) {
            int i = index.row() - 6;
            if (role==NameRole) return tr("Spectrum %1").arg(i); else return spectrum[i];
        } else {
            int i = index.row() - 6 - spectrum.size();
            if (role==NameRole) return tr("Voltage %1").arg(i); else return voltage[i];
        }
    }
}

bool ScopeColors::setData(const QModelIndex& modelindex, const QVariant& value, int role)
{
    if (role != ColorRole) {
        qWarning() << "Only color can be set!";
        return false;
    }

    unsigned index = modelindex.row();
    QColor color = value.value<QColor>();
    switch(index) {
        case 0: axes = color; emit axesChanged(); break;
        case 1: background = color; emit backgroundChanged(); break;
        case 2: border = color; emit borderChanged(); break;
        case 3: grid = color; emit gridChanged(); break;
        case 4: markers = color; emit markersChanged(); break;
        case 5: text = color; emit textChanged(); break;
    default:
        if (index<6+spectrum.size()) {
            int i = index - 6;
            spectrum[i] = color; break;
        } else {
            int i = index - 6 - spectrum.size();
            voltage[i] = color; break;
        }
    }
    writeColors();
    emit dataChanged(modelindex, modelindex);
    return true;
}

QHash<int, QByteArray> ScopeColors::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[NameRole] = "name";
    roles[ColorRole] = "color";
    return roles;
}


void ScopeColors::channelsChanged(int channels)
{
    m_channels = channels;
    readColors();
}

void ScopeColors::readColors()
{
    spectrum.resize(m_channels);
    voltage.resize(m_channels);
    beginResetModel();
    QSettings s;
    s.beginGroup("color_"+m_type);
    if (m_type == "print") {
        axes = s.value("axes",            QColor(0x00, 0x00, 0x00, 0xbf)).value<QColor>();
        background = s.value("background",QColor(0x00, 0x00, 0x00, 0x00)).value<QColor>();
        border = s.value("border",        QColor(0x00, 0x00, 0x00, 0xff)).value<QColor>();
        grid = s.value("grid",            QColor(0x00, 0x00, 0x00, 0x7f)).value<QColor>();
        markers = s.value("markers",      QColor(0x00, 0x00, 0x00, 0xef)).value<QColor>();
        text = s.value("text",            QColor(0x00, 0x00, 0x00, 0xff)).value<QColor>();
        for (unsigned channel = 0; channel < m_channels; ++channel) {
            QColor base = QColor::fromHsv(channel * 60, 0xff, 0xff);
            voltage[channel] = s.value("voltage" +channel,base.darker(120)).value<QColor>();
            spectrum[channel] = s.value("spectrum"+channel,base.darker()).value<QColor>();
        }
    } else {
        axes = s.value("axes",            QColor(0xff, 0xff, 0xff, 0x7f)).value<QColor>();
        background = s.value("background",QColor(0x00, 0x00, 0x00, 0xff)).value<QColor>();
        border = s.value("border",        QColor(0xff, 0xff, 0xff, 0xff)).value<QColor>();
        grid = s.value("grid",            QColor(0xff, 0xff, 0xff, 0x3f)).value<QColor>();
        markers = s.value("markers",      QColor(0xff, 0xff, 0xff, 0xbf)).value<QColor>();
        text = s.value("text",            QColor(0xff, 0xff, 0xff, 0xff)).value<QColor>();
        for (unsigned channel = 0; channel < m_channels; ++channel) {
            QColor base = QColor::fromHsv(channel * 60, 0xff, 0xff);
            voltage[channel] = s.value("voltage" +channel,base).value<QColor>();
            spectrum[channel] = s.value("spectrum"+channel,base.lighter()).value<QColor>();
        }
    }
    endResetModel();
}

void ScopeColors::writeColors()
{
    QSettings s;
    s.beginGroup("color_"+m_type);
    s.setValue("axes", axes);
    s.setValue("background", background);
    s.setValue("border", border);
    s.setValue("grid", grid);
    s.setValue("markers", markers);
    s.setValue("text", text);
    for (unsigned channel = 0; channel < m_channels; ++channel) {
        s.setValue("voltage"+channel, voltage[channel]);
        s.setValue("spectrum"+channel, spectrum[channel]);
    }
}


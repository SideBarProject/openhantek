#pragma once

#include <QColor>
#include <QList>
#include <QAbstractListModel>
#include <QModelIndex>

////////////////////////////////////////////////////////////////////////////////
/// OpenHantekSettingsColorValues
/// \brief Holds the color values for the oscilloscope screen.
class ScopeColors : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type WRITE setType)
    Q_PROPERTY(QColor axes READ getAxes WRITE setAxes NOTIFY axesChanged)
    Q_PROPERTY(QColor background READ getBackground WRITE setBackground NOTIFY backgroundChanged)
    Q_PROPERTY(QColor border READ getBorder WRITE setBorder NOTIFY borderChanged)
    Q_PROPERTY(QColor grid READ getGrid WRITE setGrid NOTIFY gridChanged)
    Q_PROPERTY(QColor markers READ getMarkers WRITE setMarkers NOTIFY markersChanged)
    Q_PROPERTY(QColor text READ getText WRITE setText NOTIFY textChanged)
public:
    enum Roles {
        NameRole = Qt::UserRole + 1,
        ColorRole
    };

    ScopeColors(const QString& type = QString("screen"));

    QColor axes; ///< X- and Y-axis and subdiv lines on them
    QColor background; ///< The scope background
    QColor border; ///< The border of the scope screen
    QColor grid; ///< The color of the grid
    QColor markers; ///< The color of the markers
    QColor text; ///< The default text color
    std::vector<QColor> spectrum; ///< The colors of the spectrum graphs
    std::vector<QColor> voltage; ///< The colors of the voltage graphs

    QString type() const;
    void setType(const QString& type);

    int rowCount(const QModelIndex & parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &modelindex, const QVariant &value, int role) override;

    QColor getText() const;
    void setText(const QColor& value);

    QColor getMarkers() const;
    void setMarkers(const QColor& value);

    QColor getGrid() const;
    void setGrid(const QColor& value);

    QColor getBorder() const;
    void setBorder(const QColor& value);

    QColor getBackground() const;
    void setBackground(const QColor& value);

    QColor getAxes() const;
    void setAxes(const QColor& value);

protected:
    QHash<int, QByteArray> roleNames() const override;
public slots:
    void channelsChanged(int channels);
Q_SIGNALS:
    void axesChanged();
    void backgroundChanged();
    void borderChanged();
    void gridChanged();
    void markersChanged();
    void textChanged();
private:
    unsigned m_channels = 0;
    QString m_type;
    void readColors();
    void writeColors();
};

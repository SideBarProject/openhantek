////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares the Exporter class.
//
//  Copyright (C) 2010  Oliver Haag
//  oliver.haag@gmail.com
//
//  This program is free software: you can redistribute it and/or modify it
//  under the terms of the GNU General Public License as published by the Free
//  Software Foundation, either version 3 of the License, or (at your option)
//  any later version.
//
//  This program is distributed in the hope that it will be useful, but WITHOUT
//  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//  more details.
//
//  You should have received a copy of the GNU General Public License along with
//  this program.  If not, see <http://www.gnu.org/licenses/>.
//
////////////////////////////////////////////////////////////////////////////////

#pragma once
#include <QObject>
#include <QSize>
#include <QColor>
#include <QQuickImageProvider>
#include <QQmlEngine>

#include "dataAnalyzer.h"
#include "scopecolors.h"

#include "parameters.h"

class QPaintDevice;

////////////////////////////////////////////////////////////////////////////////
///
/// \brief Exports the oscilloscope screen to a file or prints it.
class Exporter : public QObject, public QQuickImageProvider {
    Q_OBJECT
    Q_PROPERTY(QString filename READ getFilename WRITE setFilename)
    Q_PROPERTY(QSize imagesize READ getImagesize WRITE setImagesize)
    Q_PROPERTY(bool zoom READ getZoom WRITE setZoom)
    Q_PROPERTY(ScopeColors* screenColors READ getScreenColors CONSTANT)
    Q_PROPERTY(ScopeColors* printColors READ getPrintColors CONSTANT)
    public:
        Exporter(QQmlEngine& engine);

        /// Allows to request a preview image from QML by using a source url like this:
        /// Image { source: "image://exporter/increasing_number" }
        QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);

        void createDataCopy(DSOAnalyser::DataAnalyzer *dataAnalyzer);

        void setFilename(QString filename);
        QString getFilename() const;

        QSize getImagesize() const;
        void setImagesize(const QSize& size);

        bool getZoom() const;
        void setZoom(const bool zoom);

        ScopeColors* getScreenColors();
        ScopeColors* getPrintColors();

        Q_INVOKABLE void print();
        Q_INVOKABLE void exportToImage();
        Q_INVOKABLE void exportToCSV();
private:
        void draw(QPaintDevice *paintDevice, const ScopeColors& colorValues, bool forPrint);

        std::vector<DSOAnalyser::AnalyzedData> m_analyzedData;
        unsigned m_channelCount;

        QString m_filename;
        QSize   m_size         = QSize(150,150);
        bool    m_zoom         = false;
        double  m_divs_time    = 10.0; ///< Number of horizontal screen divs
        double  m_divs_voltage = 8.0; ///< Number of vertical screen divs
        unsigned m_subdivisions= 5; ///< Number of sub-divisions per div

        ScopeColors m_screen; ///< Colors for the screen
        ScopeColors m_print; ///< Colors for printout

        AnalyserSettings m_analyserSettings;
        TriggerSettings  m_triggerSettings;
        ScopeSettings    m_scopeSettings;
        std::array<ChannelSettings,MAX_CHANNELS> m_voltageSettings;
        std::array<ChannelSettings,MAX_CHANNELS> m_spectrumSettings;
};

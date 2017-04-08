////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
//  exporter.cpp
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


#include <cmath>

#include <QFile>
#include <QImage>
#include <QMutex>
#include <QPainter>
#include <QPixmap>
#include <QPrintDialog>
#include <QPrinter>
#include <QTextStream>


#include "exporter.h"
#include "dataAnalyzer.h"
#include "dataAnalyzerSettings.h"
#include "strings/dsostrings.h"
#include "strings/unitToString.h"
#include "deviceBase.h"

////////////////////////////////////////////////////////////////////////////////
// class HorizontalDock
/// \brief Initializes the printer object.
Exporter::Exporter(QQmlEngine& engine)
    : QQuickImageProvider(QQuickImageProvider::Pixmap), m_screen("screen"), m_print("print") {
    engine.addImageProvider("exporter", this);
}

QPixmap Exporter::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{
    Q_UNUSED(id);

    if (size)
        *size = m_size;
    QPixmap paintDevice(requestedSize.width() > 0 ? requestedSize.width() : m_size.width(),
                   requestedSize.height() > 0 ? requestedSize.height() : m_size.height());
    paintDevice.fill(m_screen.background);
    draw(&paintDevice, m_screen, false);
    return paintDevice;
}

void Exporter::createDataCopy(DSOAnalyser::DataAnalyzer* dataAnalyzer) {
    dataAnalyzer->mutex().lock();
    m_analyzedData = dataAnalyzer->getAllData();
    m_analyserSettings.assign(*dataAnalyzer->getAnalyserSettings());
    m_channelCount = dataAnalyzer->getDevice()->getChannelCount();
    dataAnalyzer->mutex().unlock();
}

/// \brief Set the filename of the output file (Not used for printing).
void Exporter::setFilename(QString filename) {
    if(!filename.isEmpty())
        m_filename = filename;
}

QString Exporter::getFilename() const { return m_filename; }

QSize Exporter::getImagesize() const { return m_size; }

void Exporter::setImagesize(const QSize& size) { m_size = size; }

bool Exporter::getZoom() const { return m_zoom; }

void Exporter::setZoom(const bool zoom) { m_zoom = zoom; }

ScopeColors*Exporter::getScreenColors() { return &m_screen; }

ScopeColors*Exporter::getPrintColors() { return &m_print; }

void Exporter::print() {
    // We need a QPrinter for printing, pdf- and ps-export
    QPrinter* paintDevice = new QPrinter(QPrinter::HighResolution);
    paintDevice->setOrientation(m_zoom ? QPrinter::Portrait : QPrinter::Landscape);
    paintDevice->setPageMargins(20, 20, 20, 20, QPrinter::Millimeter);

    // Show the printing dialog
    QPrintDialog dialog(paintDevice, static_cast<QWidget *>(this->parent()));
    dialog.setWindowTitle(tr("Print oscillograph"));
    if(dialog.exec() != QDialog::Accepted) {
        delete paintDevice;
        return;
    }

    paintDevice->setOutputFileName(m_filename);
    draw(paintDevice, m_print, true);
    delete paintDevice;
}

void Exporter::exportToImage() {
    QPixmap* paintDevice = new QPixmap(m_size);
    paintDevice->fill(m_screen.background);
    draw(paintDevice, m_screen, false);
    paintDevice->save(m_filename);
    delete paintDevice;
}

/// \brief Print the document (May be a file too)
void Exporter::draw(QPaintDevice *paintDevice, const ScopeColors& colorValues, bool forPrint) {

    // Create a painter for our device
    QPainter painter(paintDevice);

    // Get line height
    QFont font;
    QFontMetrics fontMetrics(font, paintDevice);
    double lineHeight = fontMetrics.height();

    painter.setBrush(Qt::SolidPattern);

    // Draw the settings table
    double stretchBase = (double) (paintDevice->width() - lineHeight * 10) / 4;

    // Print trigger details
    painter.setPen(colorValues.voltage[m_triggerSettings.source]);
    QString levelString = UnitToString::valueToString(m_voltageSettings[m_triggerSettings.source].trigger, UnitToString::UNIT_VOLTS, 3);
    QString pretriggerString = tr("%L1%").arg((int) (m_triggerSettings.pretrigger_pos_in_s * 100 + 0.5));
    painter.drawText(QRectF(0, 0, lineHeight * 10, lineHeight),
                        tr("%1  %2  %3  %4").arg(
                        m_voltageSettings[m_triggerSettings.source].name,
                        DsoStrings::slopeString(m_triggerSettings.slope),
                        levelString,
                        pretriggerString));

    int maxSamples = m_analyzedData.at(0).samples.voltage.sample.size();

    // Print sample count
    painter.setPen(colorValues.text);
    painter.drawText(QRectF(lineHeight * 10, 0, stretchBase, lineHeight), tr("%1 S").arg(maxSamples), QTextOption(Qt::AlignRight));
    // Print samplerate
    painter.drawText(QRectF(lineHeight * 10 + stretchBase, 0, stretchBase, lineHeight), UnitToString::valueToString(m_scopeSettings.samplerate, UnitToString::UNIT_SAMPLES) + tr("/s"), QTextOption(Qt::AlignRight));
    // Print timebase
    painter.drawText(QRectF(lineHeight * 10 + stretchBase * 2, 0, stretchBase, lineHeight), UnitToString::valueToString(m_scopeSettings.timebase, UnitToString::UNIT_SECONDS, 0) + tr("/div"), QTextOption(Qt::AlignRight));
    // Print frequencybase
    painter.drawText(QRectF(lineHeight * 10 + stretchBase * 3, 0, stretchBase, lineHeight), UnitToString::valueToString(m_scopeSettings.frequencybase, UnitToString::UNIT_HERTZ, 0) + tr("/div"), QTextOption(Qt::AlignRight));

    // Draw the measurement table
    stretchBase = (double) (paintDevice->width() - lineHeight * 6) / 10;
    int channelCount = 0;
    for(unsigned channel = m_voltageSettings.size() - 1; channel >= 0; channel--) {
        if((m_voltageSettings[channel].used || m_spectrumSettings[channel].used) && m_analyzedData.size()>channel) {
            ++channelCount;
            double top = (double) paintDevice->height() - channelCount * lineHeight;

            // Print label
            painter.setPen(colorValues.voltage[channel]);
            painter.drawText(QRectF(0, top, lineHeight * 4, lineHeight),
                                m_voltageSettings[channel].name);
            // Print coupling/math mode
            if((unsigned int) channel < m_channelCount)
                painter.drawText(QRectF(lineHeight * 4, top, lineHeight * 2, lineHeight),
                                    DsoStrings::couplingString(m_voltageSettings[channel].coupling));
            else
                painter.drawText(QRectF(lineHeight * 4, top, lineHeight * 2, lineHeight),
                                    DsoStrings::mathModeString(m_analyserSettings.mathmode));

            // Print voltage gain
            painter.drawText(QRectF(lineHeight * 6, top, stretchBase * 2, lineHeight), UnitToString::valueToString(m_voltageSettings[channel].gain, UnitToString::UNIT_VOLTS, 0) + tr("/div"), QTextOption(Qt::AlignRight));
            // Print spectrum magnitude
            painter.setPen(colorValues.spectrum[channel]);
            painter.drawText(QRectF(lineHeight * 6 + stretchBase * 2, top, stretchBase * 2, lineHeight), UnitToString::valueToString(m_spectrumSettings[channel].magnitude, UnitToString::UNIT_DECIBEL, 0) + tr("/div"), QTextOption(Qt::AlignRight));

            // Amplitude string representation (4 significant digits)
            painter.setPen(colorValues.text);
            painter.drawText(QRectF(lineHeight * 6 + stretchBase * 4, top, stretchBase * 3, lineHeight), UnitToString::valueToString(m_analyzedData.at(channel).amplitude, UnitToString::UNIT_VOLTS, 4), QTextOption(Qt::AlignRight));
            // Frequency string representation (5 significant digits)
            painter.drawText(QRectF(lineHeight * 6 + stretchBase * 7, top, stretchBase * 3, lineHeight), UnitToString::valueToString(m_analyzedData.at(channel).frequency, UnitToString::UNIT_HERTZ, 5), QTextOption(Qt::AlignRight));
        }
    }

    // Draw the marker table
    double scopeHeight;
    stretchBase = (double) (paintDevice->width() - lineHeight * 10) / 4;
    painter.setPen(colorValues.text);

    // Calculate variables needed for zoomed scope
    double divs = fabs(m_scopeSettings.marker[1] - m_scopeSettings.marker[0]);
    double time = divs * m_scopeSettings.timebase;
    double zoomFactor = m_divs_time / divs;
    double zoomOffset = (m_scopeSettings.marker[0] + m_scopeSettings.marker[1]) / 2;

    if(m_zoom) {
        scopeHeight = (double) (paintDevice->height() - (channelCount + 5) * lineHeight) / 2;
        double top = 2.5 * lineHeight + scopeHeight;

        painter.drawText(QRectF(0, top, stretchBase, lineHeight), tr("Zoom x%L1").arg(m_divs_time / divs, -1, 'g', 3));

        painter.drawText(QRectF(lineHeight * 10, top, stretchBase, lineHeight), UnitToString::valueToString(time, UnitToString::UNIT_SECONDS, 4), QTextOption(Qt::AlignRight));
        painter.drawText(QRectF(lineHeight * 10 + stretchBase, top, stretchBase, lineHeight), UnitToString::valueToString(1.0 / time, UnitToString::UNIT_HERTZ, 4), QTextOption(Qt::AlignRight));

        painter.drawText(QRectF(lineHeight * 10 + stretchBase * 2, top, stretchBase, lineHeight), UnitToString::valueToString(time / m_divs_time, UnitToString::UNIT_SECONDS, 3) + tr("/div"), QTextOption(Qt::AlignRight));
        painter.drawText(QRectF(lineHeight * 10 + stretchBase * 3, top, stretchBase, lineHeight), UnitToString::valueToString(divs  * m_scopeSettings.frequencybase / m_divs_time, UnitToString::UNIT_HERTZ, 3) + tr("/div"), QTextOption(Qt::AlignRight));
    }
    else {
        scopeHeight = (double) paintDevice->height() - (channelCount + 4) * lineHeight;
        double top = 2.5 * lineHeight + scopeHeight;

        painter.drawText(QRectF(0, top, stretchBase, lineHeight), tr("Marker 1/2"));

        painter.drawText(QRectF(lineHeight * 10, top, stretchBase * 2, lineHeight), UnitToString::valueToString(time, UnitToString::UNIT_SECONDS, 4), QTextOption(Qt::AlignRight));
        painter.drawText(QRectF(lineHeight * 10 + stretchBase * 2, top, stretchBase * 2, lineHeight), UnitToString::valueToString(1.0 / time, UnitToString::UNIT_HERTZ, 4), QTextOption(Qt::AlignRight));
    }

    // Set DIVS_TIME x DIVS_VOLTAGE matrix for oscillograph
    painter.setMatrix(QMatrix((paintDevice->width() - 1) / m_divs_time, 0, 0, -(scopeHeight - 1) / m_divs_voltage, (double) (paintDevice->width() - 1) / 2, (scopeHeight - 1) / 2 + lineHeight * 1.5), false);

    // Draw the graphs
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(Qt::NoBrush);

    for(unsigned zoomed = 0; zoomed < (m_zoom ? 2 : 1); ++zoomed) {
        switch(m_scopeSettings.format) {
            case ScopeSettings::GraphFormat::TY:
                // Add graphs for channels
                for(unsigned channel = 0 ; channel < m_voltageSettings.size(); ++channel) {
                    if(m_voltageSettings[channel].used && m_analyzedData.size()>channel) {
                        painter.setPen(colorValues.voltage[channel]);

                        // What's the horizontal distance between sampling points?
                        double horizontalFactor = m_analyzedData.at(channel).samples.voltage.interval / m_scopeSettings.timebase;
                        // How many samples are visible?
                        double centerPosition, centerOffset;
                        if(zoomed) {
                            centerPosition = (zoomOffset + m_divs_time / 2) / horizontalFactor;
                            centerOffset = m_divs_time / horizontalFactor / zoomFactor / 2;
                        }
                        else {
                            centerPosition = m_divs_time / 2 / horizontalFactor;
                            centerOffset = m_divs_time / horizontalFactor / 2;
                        }
                        unsigned int firstPosition = qMax((int) (centerPosition - centerOffset), 0);
                        unsigned int lastPosition = qMin((int) (centerPosition + centerOffset), (int) m_analyzedData.at(channel).samples.voltage.sample.size() - 1);

                        // Draw graph
                        QPointF *graph = new QPointF[lastPosition - firstPosition + 1];

                        for(unsigned int position = firstPosition; position <= lastPosition; ++position)
                            graph[position - firstPosition] = QPointF(position * horizontalFactor - m_divs_time / 2, m_analyzedData.at(channel).samples.voltage.sample[position] / m_voltageSettings[channel].gain + m_voltageSettings[channel].offset);

                        painter.drawPolyline(graph, lastPosition - firstPosition + 1);
                        delete[] graph;
                    }
                }

                // Add spectrum graphs
                for (unsigned channel = 0; channel < m_spectrumSettings.size(); ++channel) {
                    if(m_spectrumSettings[channel].used && m_analyzedData.size()>channel) {
                        painter.setPen(colorValues.spectrum[channel]);

                        // What's the horizontal distance between sampling points?
                        double horizontalFactor = m_analyzedData.at(channel).samples.spectrum.interval / m_scopeSettings.frequencybase;
                        // How many samples are visible?
                        double centerPosition, centerOffset;
                        if(zoomed) {
                            centerPosition = (zoomOffset + m_divs_time / 2) / horizontalFactor;
                            centerOffset = m_divs_time / horizontalFactor / zoomFactor / 2;
                        }
                        else {
                            centerPosition = m_divs_time / 2 / horizontalFactor;
                            centerOffset = m_divs_time / horizontalFactor / 2;
                        }
                        unsigned int firstPosition = qMax((int) (centerPosition - centerOffset), 0);
                        unsigned int lastPosition = qMin((int) (centerPosition + centerOffset), (int) m_analyzedData.at(channel).samples.spectrum.sample.size() - 1);

                        // Draw graph
                        QPointF *graph = new QPointF[lastPosition - firstPosition + 1];

                        for(unsigned int position = firstPosition; position <= lastPosition; ++position)
                            graph[position - firstPosition] = QPointF(position * horizontalFactor - m_divs_time / 2, m_analyzedData.at(channel).samples.spectrum.sample[position] / m_spectrumSettings[channel].magnitude + m_spectrumSettings[channel].offset);

                        painter.drawPolyline(graph, lastPosition - firstPosition + 1);
                        delete[] graph;
                    }
                }
                break;

            case ScopeSettings::GraphFormat::XY:
                break;

            default:
                break;
        }

        // Set DIVS_TIME / zoomFactor x DIVS_VOLTAGE matrix for zoomed oscillograph
        painter.setMatrix(QMatrix((paintDevice->width() - 1) / m_divs_time * zoomFactor, 0, 0, -(scopeHeight - 1) / m_divs_voltage, (double) (paintDevice->width() - 1) / 2 - zoomOffset * zoomFactor * (paintDevice->width() - 1) / m_divs_time, (scopeHeight - 1) * 1.5 + lineHeight * 4), false);
    }

    // Draw grids
    painter.setRenderHint(QPainter::Antialiasing, false);
    for(int zoomed = 0; zoomed < (m_zoom ? 2 : 1); ++zoomed) {
        // Set DIVS_TIME x DIVS_VOLTAGE matrix for oscillograph
        painter.setMatrix(QMatrix((paintDevice->width() - 1) / m_divs_time, 0, 0, -(scopeHeight - 1) / m_divs_voltage, (double) (paintDevice->width() - 1) / 2, (scopeHeight - 1) * (zoomed + 0.5) + lineHeight * 1.5 + lineHeight * 2.5 * zoomed), false);

        // Grid lines
        painter.setPen(colorValues.grid);

        if(forPrint) {
            // Draw vertical lines
            for(int div = 1; div < m_divs_time / 2; ++div) {
                for(int dot = 1; dot < m_divs_voltage / 2 * 5; ++dot) {
                    painter.drawLine(QPointF((double) -div - 0.02, (double) -dot / 5), QPointF((double) -div + 0.02, (double) -dot / 5));
                    painter.drawLine(QPointF((double) -div - 0.02, (double) dot / 5), QPointF((double) -div + 0.02, (double) dot / 5));
                    painter.drawLine(QPointF((double) div - 0.02, (double) -dot / 5), QPointF((double) div + 0.02, (double) -dot / 5));
                    painter.drawLine(QPointF((double) div - 0.02, (double) dot / 5), QPointF((double) div + 0.02, (double) dot / 5));
                }
            }
            // Draw horizontal lines
            for(int div = 1; div < m_divs_voltage / 2; ++div) {
                for(int dot = 1; dot < m_divs_time / 2 * 5; ++dot) {
                    painter.drawLine(QPointF((double) -dot / 5, (double) -div - 0.02), QPointF((double) -dot / 5, (double) -div + 0.02));
                    painter.drawLine(QPointF((double) dot / 5, (double) -div - 0.02), QPointF((double) dot / 5, (double) -div + 0.02));
                    painter.drawLine(QPointF((double) -dot / 5, (double) div - 0.02), QPointF((double) -dot / 5, (double) div + 0.02));
                    painter.drawLine(QPointF((double) dot / 5, (double) div - 0.02), QPointF((double) dot / 5, (double) div + 0.02));
                }
            }
        }
        else {
            // Draw vertical lines
            for(int div = 1; div < m_divs_time / 2; ++div) {
                for(int dot = 1; dot < m_divs_voltage / 2 * 5; ++dot) {
                    painter.drawPoint(QPointF(-div, (double) -dot / 5));
                    painter.drawPoint(QPointF(-div, (double) dot / 5));
                    painter.drawPoint(QPointF(div, (double) -dot / 5));
                    painter.drawPoint(QPointF(div, (double) dot / 5));
                }
            }
            // Draw horizontal lines
            for(int div = 1; div < m_divs_voltage / 2; ++div) {
                for(int dot = 1; dot < m_divs_time / 2 * 5; ++dot) {
                    if(dot % 5 == 0)
                        continue;                       // Already done by vertical lines
                    painter.drawPoint(QPointF((double) -dot / 5, -div));
                    painter.drawPoint(QPointF((double) dot / 5, -div));
                    painter.drawPoint(QPointF((double) -dot / 5, div));
                    painter.drawPoint(QPointF((double) dot / 5, div));
                }
            }
        }

        // Axes
        painter.setPen(colorValues.axes);
        painter.drawLine(QPointF(-m_divs_time / 2, 0), QPointF(m_divs_time / 2, 0));
        painter.drawLine(QPointF(0, -m_divs_voltage / 2), QPointF(0, m_divs_voltage / 2));
        for(double div = 0.2; div <= m_divs_time / 2; div += 0.2) {
            painter.drawLine(QPointF(div, -0.05), QPointF(div, 0.05));
            painter.drawLine(QPointF(-div, -0.05), QPointF(-div, 0.05));
        }
        for(double div = 0.2; div <= m_divs_voltage / 2; div += 0.2) {
            painter.drawLine(QPointF(-0.05, div), QPointF(0.05, div));
            painter.drawLine(QPointF(-0.05, -div), QPointF(0.05, -div));
        }

        // Borders
        painter.setPen(colorValues.border);
        painter.drawRect(QRectF(-m_divs_time / 2, -m_divs_voltage / 2, m_divs_time, m_divs_voltage));
    }

    painter.end();
}

//unsigned Exporter::getSubdivisions() const
//{
//    return m_subdivisions;
//}

//void Exporter::setSubdivisions(const unsigned& subdivisions)
//{
//    m_subdivisions = subdivisions;
//}

//double Exporter::getDivs_voltage() const
//{
//    return m_divs_voltage;
//}

//void Exporter::setDivs_voltage(double divs_voltage)
//{
//    m_divs_voltage = divs_voltage;
//}

//double Exporter::getDivs_time() const
//{
//    return m_divs_time;
//}

//void Exporter::setDivs_time(double divs_time)
//{
//    m_divs_time = divs_time;
//}

//std::vector<double> Exporter::getMarker() const
//{
//    return m_marker;
//}

//void Exporter::setMarker(const std::vector<double>& marker)
//{
//    m_marker = marker;
//}

//double Exporter::getSamplerate() const
//{
//    return m_samplerate;
//}

//void Exporter::setSamplerate(double samplerate)
//{
//    m_samplerate = samplerate;
//}

//double Exporter::getTimebase() const
//{
//    return m_timebase;
//}

//void Exporter::setTimebase(double timebase)
//{
//    m_timebase = timebase;
//}

//double Exporter::getFrequencybase() const
//{
//    return m_frequencybase;
//}

//void Exporter::setFrequencybase(double frequencybase)
//{
//    m_frequencybase = frequencybase;
//}

//GraphFormat Exporter::getFormat() const
//{
//    return m_format;
//}

//void Exporter::setFormat(const GraphFormat& format)
//{
//    m_format = format;
//}

void Exporter::exportToCSV() {
    QFile csvFile(m_filename);
    if(!csvFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream csvStream(&csvFile);

    for(unsigned channel = 0 ; channel < m_voltageSettings.size(); ++channel) {
        if(m_analyzedData.size()<=channel) continue;

        if(m_voltageSettings[channel].used) {
            // Start with channel name and the sample interval
            csvStream << "\"" << m_voltageSettings[channel].name << "\"," << m_analyzedData.at(channel).samples.voltage.interval;

            // And now all sample values in volts
            for(unsigned int position = 0; position < m_analyzedData.at(channel).samples.voltage.sample.size(); ++position)
                csvStream << "," << m_analyzedData.at(channel).samples.voltage.sample[position];

            // Finally a newline
            csvStream << '\n';
        }

        if(m_spectrumSettings[channel].used) {
            // Start with channel name and the sample interval
            csvStream << "\"" << m_spectrumSettings[channel].name << "\"," << m_analyzedData.at(channel).samples.spectrum.interval;

            // And now all magnitudes in dB
            for(unsigned int position = 0; position < m_analyzedData.at(channel).samples.spectrum.sample.size(); ++position)
                csvStream << "," << m_analyzedData.at(channel).samples.spectrum.sample[position];

            // Finally a newline
            csvStream << '\n';
        }
    }
    csvFile.close();
}

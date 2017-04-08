/**<
 * \brief Contains structures for various scope settings.
 * This file defines all settings structures for the scope view
 * each one as QObject based structure with properties so that
 * all member variables are accessable from the Qt QML Gui.
 */
#pragma once

#include "dataAnalyzerSettings.h"
#include <QObject>

namespace DSO {
    class DeviceBase;
}

//////////////////////////////////////////////////////////////////////////////
/// \enum ChannelMode
/// \brief The channel display modes.
enum ChannelMode {
    CHANNELMODE_VOLTAGE,                ///< Standard voltage view
    CHANNELMODE_SPECTRUM               ///< Spectrum view
};

////////////////////////////////////////////////////////////////////////////////
/// \struct OpenHantekSettingsScopeHorizontal
/// \brief Holds the settings for the horizontal axis.
struct ScopeSettings {
    Q_GADGET
public:
    Q_PROPERTY(GraphFormat format MEMBER format)
    Q_PROPERTY(InterpolationMode interpolation MEMBER interpolation)
    Q_PROPERTY(double frequencybase MEMBER frequencybase)
    Q_PROPERTY(double timebase MEMBER timebase)
    Q_PROPERTY(unsigned recordLength MEMBER recordLength)
    Q_PROPERTY(double samplerate MEMBER samplerate)
    Q_PROPERTY(QList<double> marker MEMBER marker)
    Q_PROPERTY(double divs_time MEMBER divs_time)
    Q_PROPERTY(unsigned divs_subdivisions MEMBER divs_subdivisions)

    //////////////////////////////////////////////////////////////////////////////
    /// \enum GraphFormat                                                    dso.h
    /// \brief The possible viewing formats for the graphs on the scope.
    enum class GraphFormat {
        TY,                     ///< The standard mode
        XY,                     ///< CH1 on X-axis, CH2 on Y-axis
        COUNT                   ///< The total number of formats
    };
    Q_ENUM(GraphFormat);

    ////////////////////////////////////////////////////////////////////////////////
    /// \enum InterpolationMode
    /// \brief The different interpolation modes for the graphs.
    enum InterpolationMode {
        INTERPOLATION_OFF = 0,              ///< Just dots for each sample
        INTERPOLATION_LINEAR,               ///< Sample dots connected by lines
        INTERPOLATION_SINC,                 ///< Smooth graph through the dots
        INTERPOLATION_COUNT                 ///< Total number of interpolation modes
    };
    Q_ENUM(InterpolationMode);

    GraphFormat format;   ///< Graph drawing mode of the scope
    InterpolationMode interpolation; ///< Interpolation mode for the graph
    double frequencybase; ///< Spectrum: Frequencybase in Hz/div
    double timebase;      ///< Voltage: Timebase in s/div
    unsigned recordLength;///< Sample count
    double samplerate;    ///< The samplerate of the oscilloscope in samples/s
    QList<double> marker; ///< Marker positions in div; -1 if disabled
    double  divs_time    = 10.0; ///< Number of horizontal screen divs
    unsigned divs_subdivisions= 5; ///< Number of sub-divisions per div
};

////////////////////////////////////////////////////////////////////////////////
/// \struct OpenHantekSettingsScopeVoltage
/// \brief Holds the settings for the normal voltage or spectrum graphs.
struct ChannelSettings : public DSO::dsoSettingsChannel {
    Q_GADGET
    Q_PROPERTY(QString name MEMBER name)
    Q_PROPERTY(double trigger MEMBER trigger)
    Q_PROPERTY(double gain MEMBER gain)
    Q_PROPERTY(double magnitude MEMBER magnitude)
    public:
    union {
        double gain;      ///< The vertical resolution in V/div
        double magnitude; ///< The vertical resolution in dB/div
    };
    QString name;    ///< Name of this channel
    double  trigger; ///< Trigger level in V
};

struct TriggerSettings: public DSO::dsoSettingsTrigger {
    Q_GADGET
    Q_PROPERTY(double pretrigger_pos_in_s MEMBER pretrigger_pos_in_s)
    Q_PROPERTY(unsigned point MEMBER point)
    Q_PROPERTY(DSO::TriggerMode mode MEMBER mode)
    Q_PROPERTY(DSO::Slope slope MEMBER slope)
    Q_PROPERTY(bool special MEMBER special)
    Q_PROPERTY(unsigned source MEMBER source)
    Q_PROPERTY(QList<double> levels READ getLevels CONSTANT)
    QList<double> getLevels() {
        QList<double> r;
        r << level[0] << level[1];
        return r;
    }

};

struct AnalyserSettings : public DSOAnalyser::AnalyserSettings {
    Q_GADGET
    public:
    void assign(const DSOAnalyser::AnalyserSettings& d) {
        spectrumEnabled = d.spectrumEnabled;
        mathChannelEnabled = d.mathChannelEnabled;
        mathmode = d.mathmode;
        spectrumWindow = d.spectrumWindow;
        spectrumReference = d.spectrumReference;
        spectrumLimit = d.spectrumLimit;
    }
    //Q_ENUMS(DSOAnalyser::MathMode)
    //Q_ENUMS(DSOAnalyser::WindowFunction)
    Q_PROPERTY(std::vector<bool> spectrumEnabled MEMBER spectrumEnabled)
    Q_PROPERTY(bool mathChannelEnabled MEMBER mathChannelEnabled)
    Q_PROPERTY(DSOAnalyser::MathMode mathmode MEMBER mathmode)
    Q_PROPERTY(DSOAnalyser::WindowFunction spectrumWindow MEMBER spectrumWindow)
    Q_PROPERTY(double spectrumReference MEMBER spectrumReference)
    Q_PROPERTY(double spectrumLimit MEMBER spectrumLimit)
};

#include <QString>
namespace DsoStrings {
    QString channelModeString(ChannelMode mode);
    QString interpolationModeString(ScopeSettings::InterpolationMode interpolation);
    /// \brief Return string representation of the given graph format.
    /// \param format The ::GraphFormat that should be returned as string.
    /// \return The string that should be used in labels etc.
    QString graphFormatString(ScopeSettings::GraphFormat format);
}

#pragma once

#include <vector>
#include <array>

namespace DSO {
    class ControlSamplerateLimits;
    #define MAX_CHANNELS 6
    #define MARKER_COUNT 2 ///< Number of markers
    #define DIVS_VOLTAGE 8 ///< no of vertical divisions

    //////////////////////////////////////////////////////////////////////////////
    /// \enum Slope                                                          dso.h
    /// \brief The slope that causes a trigger.
    enum Slope {
        SLOPE_POSITIVE,                     ///< From lower to higher voltage
        SLOPE_NEGATIVE,                     ///< From higher to lower voltage
        SLOPE_COUNT                         ///< Total number of trigger slopes
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum ChannelMode                                                    dso.h
    /// \brief The channel display modes.
    enum ChannelMode {
        CHANNELMODE_VOLTAGE,                ///< Standard voltage view
        CHANNELMODE_SPECTRUM,               ///< Spectrum view
        CHANNELMODE_COUNT                   ///< The total number of modes
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum TriggerMode                                                    dso.h
    /// \brief The different triggering modes.
    enum class TriggerMode {
        UNDEFINED = -1,
        TRIGGERMODE_AUTO,                   ///< Automatic without trigger event
        TRIGGERMODE_NORMAL,                 ///< Normal mode
        TRIGGERMODE_SINGLE                  ///< Stop after the first trigger event
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum Coupling                                                       dso.h
    /// \brief The coupling modes for the channels.

    enum Coupling {
        COUPLING_NONE,                      ///< Coupling makes no sense for the demo device
        COUPLING_AC,                        ///< Offset filtered out by condensator
        COUPLING_DC,                        ///< No filtering
        COUPLING_GND,                       ///< Channel is grounded
        COUPLING_COUNT                      ///< The total number of coupling modes
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum GraphFormat                                                    dso.h
    /// \brief The possible viewing formats for the graphs on the scope.
    enum GraphFormat {
        GRAPHFORMAT_TY,                     ///< The standard mode
        GRAPHFORMAT_XY,                     ///< CH1 on X-axis, CH2 on Y-axis
        GRAPHFORMAT_COUNT                   ///< The total number of formats
    };

    ////////////////////////////////////////////////////////////////////////////////
    /// \enum InterpolationMode                                                dso.h
    /// \brief The different interpolation modes for the graphs.
    enum InterpolationMode {
        INTERPOLATION_OFF = 0,              ///< Just dots for each sample
        INTERPOLATION_LINEAR,               ///< Sample dots connected by lines
        INTERPOLATION_SINC,                 ///< Smooth graph through the dots
        INTERPOLATION_COUNT                 ///< Total number of interpolation modes
    };
    ////////////////////////////////////////////////////////////////////////////////
    /// \enum SamplingRates                                                    dso.h
    /// \brief Sampling rates available on the hardware
    enum HWSamplingRateID {
        SAMPLING_48MHZ = 48,  /*!< 48MSa per channel */
        SAMPLING_30MHZ = 30,  /*!< 30MSa per channel */
        SAMPLING_24MHZ = 24,  /*!< 24MSa per channel */
        SAMPLING_16MHZ = 16,  /*!< 16MSa per channel */
        SAMPLING_8MHZ  =  8,  /*!<  8MSa per channel */
        SAMPLING_4MHZ  =  4,  /*!<  4MSa per channel */
        SAMPLING_1MHZ  =  1,  /*!<  1MSa per channel */
        SAMPLING_500KHZ= 50,  /*!< 500kSa per channel */
        SAMPLING_200KHZ= 20,  /*!< 200kSa per channel */
        SAMPLING_100KHZ= 10   /*!< 100kSa per channel */
    };

    enum HWRecordLengthID {
        RECORDLENGTH_1KB   = 1024,
        RECORDLENGTH_2KB   = 2048,
        RECORDLENGTH_5KB   = 5120,
        RECORDLENGTH_10KB  = 10240,
        RECORDLENGTH_20KB  = 20480,
        RECORDLENGTH_INVALID = -1
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettingsSamplerate
    /// \brief Stores the current samplerate settings of the device.
    struct dsoSettingsSamplerate {
//        double target_samplerate;        ///< The target samplerate set via setSamplerate
//        ControlSamplerateLimits *limits; ///< The samplerate limits
        double timebase;
        HWRecordLengthID recordLengthID;
        unsigned int downsampler  = 1;   ///< The variable downsampling factor
        double current            = 1e8; ///< The current samplerate
    };


    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettingsTrigger
    /// \brief Stores the current trigger settings of the device.
    struct dsoSettingsTrigger {
        std::array<double,MAX_CHANNELS> level; ///< The trigger level for each channel in V
        double pretrigger_pos_in_s= 0;///< The current pretrigger position in s
        unsigned int point  = 0;      ///< The trigger position in Hantek coding
        TriggerMode mode    = TriggerMode::TRIGGERMODE_NORMAL; ///< The trigger mode
        Slope slope         = Slope::SLOPE_POSITIVE; ///< The trigger slope
        bool special        = false;  ///< true, if the trigger source is special
        unsigned int source = 0;      ///< The trigger source
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettingsVoltage
    /// \brief Stores the current amplification settings of the device.
    struct dsoSettingsChannel {
        unsigned int gainID = 0;   ///< The gain id (@see DSO::dsoSpecification.gainLevel[gainID]
        double gain;               ///< The gain in V/div
        int misc; ///< Different enums, coupling for real- and mode for math-channels
        std::string name;          ///< Name of this channel
        double offset     = 0.0;   ///< The screen offset for each channel
        double offsetReal = 0.0;   ///< The real offset for each channel (Due to quantization)
        double trigger;            ///< Trigger level in V
        bool used         = false; ///< true, if the channel is used
        Coupling coupling = Coupling::COUPLING_DC; ///< The coupling of this channel
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettings
    /// \brief Stores the current settings of the device.
    struct dsoSettings {
        dsoSettingsSamplerate samplerate;    ///< The samplerate settings
        std::array<dsoSettingsChannel,MAX_CHANNELS> voltage; ///< The amplification settings
        dsoSettingsTrigger trigger;          ///< The trigger settings
        double timebase;
        unsigned int recordTypeID     = 1;   ///< The id in the record type array (@see DSO::dsoSpecification and samplerate_single/multi)
        unsigned short int usedChannels = 0; ///< Number of activated channels
    };
}

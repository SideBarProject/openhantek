#pragma once

#include <vector>
#include <array>

namespace DSO {
    class ControlSamplerateLimits;
    #define MAX_CHANNELS 6

        //////////////////////////////////////////////////////////////////////////////
    /// \enum Slope                                                          dso.h
    /// \brief The slope that causes a trigger.
    enum class Slope {
        POSITIVE,                     ///< From lower to higher voltage
        NEGATIVE                     ///< From higher to lower voltage
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum TriggerMode                                                    dso.h
    /// \brief The different triggering modes.
    enum class TriggerMode {
        UNDEFINED = -1,
        AUTO,                   ///< Automatic without trigger event
        NORMAL,                 ///< Normal mode
        SINGLE                 ///< Stop after the first trigger event
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum Coupling                                                       dso.h
    /// \brief The coupling modes for the channels.
    enum class Coupling {
        AC,                        ///< Offset filtered out by condensator
        DC,                        ///< No filtering
        GND                       ///< Channel is grounded
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettingsSamplerate
    /// \brief Stores the current samplerate settings of the device.
    struct dsoSettingsSamplerate {
        double target_samplerate;        ///< The target samplerate set via setSamplerate
        ControlSamplerateLimits *limits; ///< The samplerate limits
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
        TriggerMode mode    = TriggerMode::NORMAL; ///< The trigger mode
        Slope slope         = Slope::POSITIVE; ///< The trigger slope
        bool special        = false;  ///< true, if the trigger source is special
        unsigned int source = 0;      ///< The trigger source
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettingsVoltage
    /// \brief Stores the current amplification settings of the device.
    struct dsoSettingsChannel {
        unsigned int gainID = 0;  ///< The gain id (@see DSO::dsoSpecification.gainLevel[gainID])
        double offset     = 0.0;  ///< The screen offset for each channel
        double offsetReal = 0.0;  ///< The real offset for each channel (Due to quantization)
        bool used         = false;///< true, if the channel is used
        Coupling coupling = Coupling::DC; ///< The coupling of this channel
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSettings
    /// \brief Stores the current settings of the device.
    struct dsoSettings {
        dsoSettingsSamplerate samplerate;    ///< The samplerate settings
        std::array<dsoSettingsChannel,MAX_CHANNELS> voltage; ///< The amplification settings
        dsoSettingsTrigger trigger;          ///< The trigger settings
        unsigned int recordTypeID     = 1;   ///< The id in the record type array (@see DSO::dsoSpecification and samplerate_single/multi)
        unsigned short int usedChannels = 0; ///< Number of activated channels
    };
}

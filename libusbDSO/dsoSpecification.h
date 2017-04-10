#pragma once

/// \todo Make channels configurable

#include <vector>
#include "dsoSettings.h"

namespace DSO {
    #define MAX_CHANNELS 6

    struct dsoShortMinMax {
        unsigned short int minimum = 0x0000;
        unsigned short int maximum = 0xffff;
        void setMinChangeEndianess(unsigned short int m) {
            minimum = (m << 8) | ((m & 0xff00) >> 8);
        }
        void setMaxChangeEndianess(unsigned short int m) {
            maximum = (m << 8) | ((m & 0xff00) >> 8);
        }
        dsoShortMinMax() = default;
        dsoShortMinMax(unsigned short int minimum, unsigned short int maximum) : minimum(minimum), maximum(maximum) {}
    };

    struct dsoRecord {
        unsigned length_per_channel = 0; ///< Record length, 0 means rolling
        unsigned divider = 0; ///< Samplerate dividers for record lengths. Normally 1, 1000 for rolling
        dsoRecord(unsigned length, unsigned divider) : length_per_channel(length), divider(divider) {}
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct ControlSamplerateLimits
    /// \brief Stores the samplerate limits for calculations.
    struct ControlSamplerateLimits {
        double base = 0; ///< The base for sample rate calculations
        double max  = 0; ///< The maximum sample rate
        unsigned maxDownsampler = 0; ///< The maximum downsampling ratio
        std::vector<dsoRecord> recordTypes;
    };

    struct dsoGainLevel {
        /// The index of the selected gain on the hardware
        unsigned char gainIndex;
        /// Available voltage steps in V/screenheight
        double gainSteps;
        /// The sample values at the top of the screen
        unsigned short int voltage;
        // Calibration per channel
        dsoShortMinMax offset[MAX_CHANNELS];

        dsoGainLevel(unsigned char gainIndex, double gainSteps, unsigned short int voltage)
            : gainIndex(gainIndex), gainSteps(gainSteps), voltage(voltage) {}
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoAvailableSamplingRate
    /// \brief Stores the samplerates available on the device.
    ///
    struct dsoAvailableSamplingRate {
        HWSamplingRateID samplingRateID;    ///< this is the command to be sent to the scope
        double samplingrateValue;
        HWRecordLengthID recordLengthID;
        double timeBase;
        unsigned int downsampling;
        dsoAvailableSamplingRate(HWSamplingRateID samplingRateID, double samplingrateValue, HWRecordLengthID recordLengthID, double timeBase, unsigned downSampling)
            : samplingRateID(samplingRateID), samplingrateValue(samplingrateValue), recordLengthID(recordLengthID), timeBase(timeBase), downsampling(downSampling){}
    };

    enum dsoFeatures {
        noFeatures = 0,
        hasHardwareTrigger = 1
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct dsoSpecification
    /// \brief Stores the specifications of the currently connected device.
    struct dsoSpecification {
        ControlSamplerateLimits samplerate_single; ///< The limits for single channel mode
        ControlSamplerateLimits samplerate_multi; ///< The limits for multi channel mode
        unsigned char sampleSize  = 8; ///< Number of bits per sample. Default: 8bit ADC

        unsigned channels         = 0;
        unsigned channels_special = 0;
        std::vector<std::string> specialTriggerSources; ///< Names of the special trigger sources
        std::vector<Coupling> availableCoupling;
        dsoFeatures features = noFeatures;
        std::vector<dsoAvailableSamplingRate> availableSamplingRates;
        std::vector<DSO::HWRecordLengthID> availableRecordLengths;
        std::vector<dsoGainLevel> gainLevel;
    };
}

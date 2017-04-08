////////////////////////////////////////////////////////////////////////////////
//
//  OpenHantek
/// \brief Declares types needed for the Hantek::Device class.
//
//  Copyright (C) 2008, 2009  Oleg Khudyakov
//  prcoder@potrebitel.ru
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
#include <stdint.h>

#include "protocolBulk.h"
#include "protocolControl.h"

////////////////////////////////////////////////////////////////////////////////
/// \namespace Hantek
/// \brief All %Hantek DSO device specific things.
namespace Hantek2xxx_5xxx {
    //////////////////////////////////////////////////////////////////////////////
    /// \enum UsedChannels
    /// \brief The enabled channels.
    enum UsedChannels {
        USED_CH1,    ///< Only channel 1 is activated
        USED_CH2,    ///< Only channel 2 is activated
        USED_CH1CH2, ///< Channel 1 and 2 are both activated
        USED_NONE    ///< No channels are activated
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum TriggerSource
    /// \brief The possible trigger sources.
    enum TriggerSource {
        TRIGGER_CH2, TRIGGER_CH1,
        TRIGGER_ALT,
        TRIGGER_EXT, TRIGGER_EXT10
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum RecordLengthId
    /// \brief The size id for CommandSetTriggerAndSamplerate.
    enum RecordLengthId {
        RECORDLENGTHID_ROLL = 0, ///< Used for the roll mode
        RECORDLENGTHID_SMALL, ///< The standard buffer with 10240 samples
        RECORDLENGTHID_LARGE ///< The large buffer, 32768 samples (14336 for DSO-5200)
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum BUsedChannels
    /// \brief The enabled channels for the DSO-2250.
    enum BUsedChannels {
        BUSED_CH1,    ///< Only channel 1 is activated
        BUSED_NONE,   ///< No channels are activated
        BUSED_CH1CH2, ///< Channel 1 and 2 are both activated
        BUSED_CH2     ///< Only channel 2 is activated
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum DTriggerPositionUsed
    /// \brief The trigger position states for the 0x0d command.
    enum DTriggerPositionUsed {
        DTRIGGERPOSITION_OFF = 0, ///< Used for Roll mode
        DTRIGGERPOSITION_ON = 7 ///< Used for normal operation
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct FilterBits
    /// \brief The bits for BULK_SETFILTER.
    struct FilterBits {
        uint8_t channel1:1; ///< Set to true when channel 1 isn't used
        uint8_t channel2:1; ///< Set to true when channel 2 isn't used
        uint8_t trigger:1; ///< Set to true when trigger isn't used
        uint8_t reserved:5; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct GainBits
    /// \brief The gain bits for BULK_SETGAIN.
    struct GainBits {
        uint8_t channel1:2; ///< Gain for CH1, 0 = 1e* V, 1 = 2e*, 2 = 5e*
        uint8_t channel2:2; ///< Gain for CH1, 0 = 1e* V, 1 = 2e*, 2 = 5e*
        uint8_t reserved:4; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct Tsr1Bits
    /// \brief Trigger and samplerate bits (Byte 1).
    struct Tsr1Bits {
        uint8_t triggerSource:2; ///< The trigger source, see Hantek::TriggerSource
        uint8_t recordLength:3; ///< See ::RecordLengthId
        uint8_t samplerateId:2; ///< Samplerate ID when downsampler is disabled
        uint8_t downsamplingMode:1; ///< true, if Downsampler is used
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct Tsr2Bits
    /// \brief Trigger and samplerate bits (Byte 2).
    struct Tsr2Bits {
        uint8_t usedChannels:2; ///< Used channels, see Hantek::UsedChannels
        uint8_t fastRate:1; ///< true, if one channels uses all buffers
        uint8_t triggerSlope:1; ///< The trigger slope, see Dso::Slope, inverted when Tsr1Bits.samplerateFast is uneven
        uint8_t reserved:4; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct CTriggerBits
    /// \brief Trigger bits for 0x0c command.
    struct CTriggerBits {
        uint8_t triggerSource:2; ///< The trigger source, see Hantek::TriggerSource
        uint8_t triggerSlope:1; ///< The trigger slope, see Dso::Slope
        uint8_t reserved:5; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct DBufferBits
    /// \brief Buffer mode bits for 0x0d command.
    struct DBufferBits {
        uint8_t triggerPositionUsed:3; ///< See ::DTriggerPositionUsed
        uint8_t recordLength:3; ///< See ::RecordLengthId
        uint8_t reserved:2; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct ESamplerateBits
    /// \brief Samplerate bits for DSO-2250 0x0e command.
    struct ESamplerateBits {
        uint8_t fastRate:1; ///< false, if one channels uses all buffers
        uint8_t downsampling:1; ///< true, if the downsampler is activated
        uint8_t reserved:4; ///< Unused bits
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \struct ETsrBits
    /// \brief Trigger and samplerate bits for DSO-5200/DSO-5200A 0x0e command.
    struct ETsrBits {
        uint8_t fastRate:1; ///< false, if one channels uses all buffers
        uint8_t usedChannels:2; ///< Used channels, see Hantek::UsedChannels
        uint8_t triggerSource:2; ///< The trigger source, see Hantek::TriggerSource
        uint8_t triggerSlope:2; ///< The trigger slope, see Dso::Slope
        uint8_t triggerPulse:1; ///< Pulses are causing trigger events
    };
}

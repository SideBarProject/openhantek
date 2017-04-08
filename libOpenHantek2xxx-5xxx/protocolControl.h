#pragma once

#include "utils/transferBuffer.h"
#include "usbCommunicationQueues.h"

namespace Hantek2xxx_5xxx {
    //////////////////////////////////////////////////////////////////////////////
    /// \enum ControlCode                                           hantek/types.h
    /// \brief All supported control commands.
    enum ControlCode {
        CONTROL_UNDEFINED = -1,
        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        ///   The 0xa2 control read/write command gives access to a ::ControlValue.
        /// </p>
        /// <p><br /></p>
        CONTROL_VALUE = 0xa2,

        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        ///   The 0xb2 control read command gets the speed level of the USB connection:
        ///   <table>
        ///     <tr>
        ///       <td>::ConnectionSpeed</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///     </tr>
        ///   </table>
        /// </p>
        /// <p><br /></p>
        CONTROL_GETSPEED = 0xb2,

        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        ///   The 0xb3 control write command is sent before any bulk command:
        ///   <table>
        ///     <tr>
        ///       <td>0x0f</td>
        ///       <td>::BulkIndex</td>
        ///       <td>::BulkIndex</td>
        ///       <td>::BulkIndex</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///     </tr>
        ///   </table>
        /// </p>
        /// <p><br /></p>
        CONTROL_BEGINCOMMAND = 0xb3,

        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        ///   The 0xb4 control write command sets the channel offsets:
        ///   <table>
        ///     <tr>
        ///       <td>Ch1Offset[1]</td>
        ///       <td>Ch1Offset[0]</td>
        ///       <td>Ch2Offset[1]</td>
        ///       <td>Ch2Offset[0]</td>
        ///       <td>TriggerOffset[1]</td>
        ///       <td>TriggerOffset[0]</td>
        ///     </tr>
        ///   </table>
        ///   <table>
        ///     <tr>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///     </tr>
        ///   </table>
        /// </p>
        /// <p><br /></p>
        CONTROL_SETOFFSET = 0xb4,

        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        ///   The 0xb5 control write command sets the internal relays:
        ///   <table>
        ///     <tr>
        ///       <td>0x00</td>
        ///       <td>0x04 ^ (Ch1Gain < 1 V)</td>
        ///       <td>0x08 ^ (Ch1Gain < 100 mV)</td>
        ///       <td>0x02 ^ (Ch1Coupling == DC)</td>
        ///     </tr>
        ///   </table>
        ///   <table>
        ///     <tr>
        ///       <td>0x20 ^ (Ch2Gain < 1 V)</td>
        ///       <td>0x40 ^ (Ch2Gain < 100 mV)</td>
        ///       <td>0x10 ^ (Ch2Coupling == DC)</td>
        ///       <td>0x01 ^ (Trigger == EXT)</td>
        ///     </tr>
        ///   </table>
        ///   <table>
        ///     <tr>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///       <td>0x00</td>
        ///     </tr>
        ///   </table>
        /// </p>
        /// <p>
        ///   The limits are <= instead of < for the 10 bit models, since those support voltages up to 10 V.
        /// </p>
        /// <p><br /></p>
        CONTROL_SETRELAYS = 0xb5
    };

    //////////////////////////////////////////////////////////////////////////////
    /// \enum ControlValue                                          hantek/types.h
    /// \brief All supported values for control commands.
    enum ControlValue {
        VALUE_UNDEFINED = -1,
        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        /// Value 0x08 is the calibration data for the channels offsets. It holds the offset value for the top and bottom of the scope screen for every gain step on every channel. The data is stored as a three-dimensional array:<br />
        /// <i>channelLevels[channel][GainId][::LevelOffset]</i>
        /// </p>
        /// <p><br /></p>
        VALUE_OFFSETLIMITS = 0x08,

        /// <em>[::MODEL_DSO2090, ::MODEL_DSO2150, ::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        /// Value 0x0a is the address of the device. It has a length of one byte.
        /// </p>
        /// <p><br /></p>
        VALUE_DEVICEADDRESS = 0x0a,

        /// <em>[::MODEL_DSO2250, ::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        /// Value 0x60 is the calibration data for the fast rate mode on the DSO-2250, DSO-5200 and DSO-5200A. It's used to correct the level differences between the two merged channels to avoid deterministic noise.
        /// </p>
        /// <p><br /></p>
        VALUE_FASTRATECALIBRATION = 0x60,

        /// <em>[::MODEL_DSO5200, ::MODEL_DSO5200A]</em>
        /// <p>
        /// Value 0x70 contains correction values for the ETS functionality of the DSO-5200 and DSO-5200A.
        /// </p>
        /// <p><br /></p>
        VALUE_ETSCORRECTION = 0x70
    };

    //////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief The CONTROL_BEGINCOMMAND builder.
    class ControlBeginCommand : public USBTransferBuffer, public ControlUSB {
        public:
            //////////////////////////////////////////////////////////////////////////////
            /// \enum BulkIndex
            /// \brief Can be set by CONTROL_BEGINCOMMAND, maybe it allows multiple commands at the same time?
            enum BulkIndex {
                COMMANDINDEX_0 = 0x03, ///< Used most of the time
                COMMANDINDEX_1 = 0x0a,
                COMMANDINDEX_2 = 0x09,
                COMMANDINDEX_3 = 0x01, ///< Used for ::BULK_SETTRIGGERANDSAMPLERATE sometimes
                COMMANDINDEX_4 = 0x02,
                COMMANDINDEX_5 = 0x08
            };
            ControlBeginCommand(BulkIndex index = COMMANDINDEX_0);

            BulkIndex getIndex();
            void setIndex(BulkIndex index);

        private:
            void init();
    };

    //////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief The CONTROL_SETOFFSET builder.
    class ControlSetOffset : public USBTransferBuffer, public ControlUSB {
        public:
            ControlSetOffset();
            ControlSetOffset(uint16_t channel1, uint16_t channel2, uint16_t trigger);

            uint16_t getChannel(unsigned int channel);
            ControlSetOffset& setChannel(unsigned int channel, uint16_t offset);
            uint16_t getTrigger();
            ControlSetOffset& setTrigger(uint16_t level);

        private:
            void init();
    };

    //////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief The CONTROL_SETRELAYS builder.
    class ControlSetRelays : public USBTransferBuffer, public ControlUSB {
        public:
            ControlSetRelays(bool ch1Below1V = false, bool ch1Below100mV = false, bool ch1CouplingDC = false, bool ch2Below1V = false, bool ch2Below100mV = false, bool ch2CouplingDC = false, bool triggerExt = false);

            bool getBelow1V(unsigned int channel);
            ControlSetRelays& setBelow1V(unsigned int channel, bool below);
            bool getBelow100mV(unsigned int channel);
            ControlSetRelays& setBelow100mV(unsigned int channel, bool below);
            bool getCoupling(unsigned int channel);
            ControlSetRelays& setCoupling(unsigned int channel, bool dc);
            bool getTrigger();
            ControlSetRelays& setTrigger(bool ext);
    };
}

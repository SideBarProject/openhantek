#pragma once

#include "dsoSettings.h"
#include <array>

namespace DSOAnalyser {

//////////////////////////////////////////////////////////////////////////////
/// \enum WindowFunction                                                 dso.h
/// \brief The supported window functions.
/// These are needed for spectrum analysis and are applied to the sample values
/// before calculating the DFT.
enum WindowFunction {
    WINDOW_UNDEFINED = -1,
    WINDOW_RECTANGULAR = 0,                 ///< Rectangular window (aka Dirichlet)
    WINDOW_HAMMING,                     ///< Hamming window
    WINDOW_HANN,                        ///< Hann window
    WINDOW_COSINE,                      ///< Cosine window (aka Sine)
    WINDOW_LANCZOS,                     ///< Lanczos window (aka Sinc)
    WINDOW_BARTLETT,                    ///< Bartlett window (Endpoints == 0)
    WINDOW_TRIANGULAR,                  ///< Triangular window (Endpoints != 0)
    WINDOW_GAUSS,                       ///< Gauss window (simga = 0.4)
    WINDOW_BARTLETTHANN,                ///< Bartlett-Hann window
    WINDOW_BLACKMAN,                    ///< Blackman window (alpha = 0.16)
    //WINDOW_KAISER,                      ///< Kaiser window (alpha = 3.0)
    WINDOW_NUTTALL,                     ///< Nuttall window, cont. first deriv.
    WINDOW_BLACKMANHARRIS,              ///< Blackman-Harris window
    WINDOW_BLACKMANNUTTALL,             ///< Blackman-Nuttall window
    WINDOW_FLATTOP,                     ///< Flat top window
    WINDOW_COUNT                        ///< Total number of window functions
};

//////////////////////////////////////////////////////////////////////////////
/// \enum MathMode                                                       dso.h
/// \brief The different math modes for the math-channel.
enum class MathMode {
    ADD_CH1_CH2,                     ///< Add the values of the channels
    SUB_CH2_FROM_CH1,                     ///< Subtract CH2 from CH1
    SUB_CH1_FROM_CH2                     ///< Subtract CH1 from CH2
};

////////////////////////////////////////////////////////////////////////////////
/// \struct OpenHantekSettingsScope                                          settings.h
/// \brief Holds the settings for the oscilloscope.
struct AnalyserSettings {
    std::vector<bool> spectrumEnabled;
    bool mathChannelEnabled      = false;
    MathMode mathmode;
    WindowFunction spectrumWindow = WINDOW_RECTANGULAR; ///< Window function for DFT
    double spectrumReference      = 0.0; ///< Reference level for spectrum in dBm
    double spectrumLimit          = 1.0; ///< Minimum magnitude of the spectrum (Avoids peaks)
};

}

#pragma once

//////////////////////////////////////////////////////////////////////////////
/// \enum ErrorCode
/// \brief The return codes for device control methods.
enum class ErrorCode {
    ERROR_NONE        = 0,      ///< Successful operation
    ERROR_CONNECTION  = -10000, ///< Device not connected or communication error
    ERROR_UNSUPPORTED = -10001, ///< Not supported by this device
    ERROR_PARAMETER   = -10002, ///< Parameter out of range
    ERROR_WRONG_DRIVER= -10004  ///< Device type not supported
};

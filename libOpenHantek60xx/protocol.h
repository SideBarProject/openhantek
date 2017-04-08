#pragma once

#define HT6022_IR1_REQUEST_TYPE             0X40
#define HT6022_IR1_REQUEST                  0XE0
#define HT6022_IR1_VALUE                    0X00
#define HT6022_IR1_INDEX                    0X00
#define HT6022_IR1_SIZE                     0X01
#define HT6022_IR2_REQUEST_TYPE             0X40
#define HT6022_IR2_REQUEST                  0XE1
#define HT6022_IR2_VALUE                    0X00
#define HT6022_IR2_INDEX                    0X00
#define HT6022_IR2_SIZE                     0X01
#define HT6022_SR_REQUEST_TYPE              0X40
#define HT6022_SR_REQUEST                   0XE2
#define HT6022_SR_VALUE                     0X00
#define HT6022_SR_INDEX                     0X00
#define HT6022_SR_SIZE                      0X01
#define HT6022_SETCALLEVEL_REQUEST_TYPE     0XC0
#define HT6022_SETCALLEVEL_REQUEST          0XA2
#define HT6022_SETCALLEVEL_VALUE            0X08
#define HT6022_SETCALLEVEL_INDEX            0X00
#define HT6022_GETCALLEVEL_REQUEST_TYPE     0X40
#define HT6022_GETCALLEVEL_REQUEST          0XA2
#define HT6022_GETCALLEVEL_VALUE            0X08
#define HT6022_GETCALLEVEL_INDEX            0X00
#define HT6022_READ_CONTROL_REQUEST_TYPE    0X40
#define HT6022_READ_CONTROL_REQUEST         0XE3
#define HT6022_READ_CONTROL_VALUE           0X00
#define HT6022_READ_CONTROL_INDEX           0X00
#define HT6022_READ_CONTROL_SIZE            0X01
#define HT6022_READ_CONTROL_DATA            0X01

/**
  * @brief Error Code
  */
enum class HT6022_Error
{
     SUCCESS             = 0,
     ERROR_INVALID_PARAM = -2,
     ERROR_ACCESS        = -3,
     ERROR_NO_DEVICE     = -4,
     ERROR_TIMEOUT       = -7,
     ERROR_NO_MEM        = -11,
     ERROR_OTHER         = -99
};

/**
  * @brief  Size of data bufffer
  */
enum class HT6022_DataSize
{
  DS_1KB   = 0x00000400, /*!< 1024 Bytes */
  DS_2KB   = 0x00000800, /*!< 2048 Bytes */
  DS_4KB   = 0x00001000, /*!< 4096 Bytes */
  DS_8KB   = 0x00002000, /*!< 8192 Bytes */
  DS_16KB  = 0x00004000, /*!< 16384 Bytes */
  DS_32KB  = 0x00008000, /*!< 32768 Bytes */
  DS_64KB  = 0x00010000, /*!< 65536 Bytes */
  DS_128KB = 0x00020000, /*!< 131072 Bytes */
  DS_256KB = 0x00040000, /*!< 262144 Bytes */
  DS_512KB = 0x00080000, /*!< 524288 Bytes */
  DS_1MB   = 0x00100000  /*!< 1048576 Bytes */
};

/**
  * @brief Size of calibration values buffer
  */
enum class HT6022_CalibrationValues
{
  CV_32B   = 0x00000010, /*!< 32 Bytes */
  CV_64B   = 0x00000020, /*!< 64 Bytes */
  CV_128B  = 0x00000080 /*!< 128 Bytes */
};

/**
  * @brief  Sample rate
  */
enum class HT6022_SampleRate
{
    SR_24MSa  = 0x30, /*!< 24MSa per channel */
    SR_16MSa  = 0x10, /*!< 16MSa per channel */
    SR_8MSa   = 0x08, /*!< 8MSa per channel */
    SR_4MSa   = 0x04, /*!< 4MSa per channel */
    SR_1MSa   = 0x01, /*!< 1MSa per channel */
    SR_500KSa = 0x32, /*!< 500KSa per channel */
    SR_200KSa = 0x14, /*!< 200KSa per channel */
    SR_100KSa = 0x0A  /*!< 100KSa per channel */
};

/**
  * @brief Input range
  */
enum class HT6022_InputRange
{
    IR_10V   = 0x01, /*!< -5V    to 5V    */
    IR_5V    = 0x02, /*!< -2.5V  to 2.5V  */
    IR_2V    = 0x05, /*!< -1V    to 1V    */
    IR_1V    = 0x0A  /*!< -500mv to 500mv */
};

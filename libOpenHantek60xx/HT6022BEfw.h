/**
  ******************************************************************************
  * @file    HT6022fw.c
  * @author  Rodrigo Pedroso Mendes
  * @email   rpm_2003_rpm@yahoo.com.br
  * @version V1.0
  * @date    04-14-2013
  * @brief   This file provides the HT6022 firmware header
  ******************************************************************************
  */

#pragma once

#define HT6022_FIRMWARE_SIZE            458
#define HT6022_FIRMWARE_REQUEST         0XA0
#define HT6022_FIRMWARE_INDEX           0X00

extern unsigned char HT6022_Firmware[];

/*******************************************************************************
 * @file    Inc/EVSDriver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   External Voltage Switcher Driver Header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __EVSDRIVER_H_
#define __EVSDRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "ADG729Driver.h"
#include "LVSConfig.h"
#include "PinMapper.h"
#include "SPIDriver.h"
#include "StatusManager.h"


/* Macros --------------------------------------------------------------------*/
#define ADG714_RSTDelay	512

/* Public function prototypes ------------------------------------------------*/
ErrorStatus EVSInit( EVSHandler *EVS_Handler);
ErrorStatus EVSReset( EVSHandler *EVS_Handler);
ErrorStatus EVSCheck( EVSHandler *VS_Handler);
ErrorStatus EVSSetSinglePin( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t Source);
ErrorStatus EVSGetSinglePin( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t *Source);
ErrorStatus EVSSetAllPins( EVSHandler *EVS_Handler, uint8_t *SourceData);
ErrorStatus EVSGetAllPins( EVSHandler *EVS_Handler, uint8_t *SourceData);
ErrorStatus EVSSetAllPinsSameSource( EVSHandler *EVS_Handler, uint8_t Source);
ErrorStatus EVSReportSourceConnection( EVSHandler *EVS_Handler, uint8_t Source, uint32_t *Number);

#ifdef __cplusplus
}
#endif

#endif /* __EVSDRIVER_H_ */


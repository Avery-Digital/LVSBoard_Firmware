/*******************************************************************************
 * @file    Src/ADG729Driver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   ADG729 Driver header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ADG729DRIVER_H_
#define __ADG729DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "I2CDriver.h"
#include "LVSConfig.h"

/* Public function prototypes ------------------------------------------------*/
ErrorStatus ADG729Init( I2CHandler* ADG729_I2C);
ErrorStatus ADG729WritePort( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Port);
ErrorStatus ADG729ReadPort( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t* Port);
ErrorStatus ADG729SetPin( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Pin);
ErrorStatus ADG729ResetPin( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Pin);
ErrorStatus ADG729SetAllPins( I2CHandler* ADG729_I2C, uint8_t Address);
ErrorStatus ADG729ResetAllPins( I2CHandler* ADG729_I2C, uint8_t Address);
ErrorStatus ADG729Check( I2CHandler* ADG729_I2C, uint8_t Address);

#ifdef __cplusplus
}
#endif

#endif /* __ADG729DRIVER_H_ */


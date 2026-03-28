/*******************************************************************************
 * @file    Inc/GPIODriver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   GPIO Driver Header
 *******************************************************************************
 * Copyright (c) 2023 Avery Digital Data
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GPIODRIVER_H
#define __GPIODRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "LVSConfig.h"

/* Public function prototypes ------------------------------------------------*/
ErrorStatus GPIOInit(GPIOHandler *GPIO_Handler);
ErrorStatus GPIOReset(GPIOHandler *GPIO_Handler);
ErrorStatus GPIOSet(GPIOHandler *GPIO_Handler);
ErrorStatus GPIOToggle(GPIOHandler *GPIO_Handler);
uint8_t GPIORead(GPIOHandler *GPIO_Handler);
ErrorStatus GPIOReadPort(GPIOHandler *GPIO_Handler, uint16_t* Data);
ErrorStatus GPIOWritePort(GPIOHandler *GPIO_Handler, uint16_t Data);

#ifdef __cplusplus
}
#endif

#endif /* __GPIODRIVER_H */


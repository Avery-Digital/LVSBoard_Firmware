/******************************************************************************
 * @file    Inc/I2CDriver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   I2C Driver header
 ******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2CDRIVER_H_
#define __I2CDRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_i2c.h"
#include "LVSConfig.h"

/* Public function prototypes ------------------------------------------------*/
ErrorStatus I2CInit (I2CHandler *I2C_Handler );
ErrorStatus I2CWriteData (I2CHandler* I2C_Handler, uint8_t I2C_Address, uint8_t* Data, uint8_t ByteNum);
ErrorStatus I2CReadData (I2CHandler* I2C_Handler, uint8_t I2C_Address, uint8_t* Data, uint8_t ByteNum);

#ifdef __cplusplus
}
#endif

#endif /* __I2CDRIVER_H_ */


/*******************************************************************************
 * @file    Src/SPIDriver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   SPI Driver header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPIDRIVER_H_
#define __SPIDRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_spi.h"
#include "LVSConfig.h"

/* Public function prototypes ------------------------------------------------*/
ErrorStatus SPIInit ( SPIHandler *SPI_Handler);
ErrorStatus SPIDataTransceive (SPIHandler* SPI_Handler, uint32_t* Data_Rx, uint32_t Data_Tx);

#ifdef __cplusplus
}
#endif

#endif /* __SPIDRIVER_H_ */


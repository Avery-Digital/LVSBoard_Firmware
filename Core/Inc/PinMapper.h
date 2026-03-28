/*******************************************************************************
 * @file    Inc/PinMapper.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Pin Mapper Header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PINMAPPER_H_
#define __PINMAPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Macros --------------------------------------------------------------------*/
#define TOTAL_SOCKET_PINS	300
#define ROWS_PER_COL		30
#define ROWS_PER_BANK		16
#define COLS_PER_BANK		10

/* Public function prototypes ------------------------------------------------*/
uint8_t PinBankMAP ( uint16_t SocketPin);
uint8_t PinBlockMAP ( uint16_t SocketPin);
uint8_t PinPositionMAP (uint16_t SocketPin);

#ifdef __cplusplus
}
#endif

#endif /* __PINMAPPER_H_ */


/*******************************************************************************
 * @file    Inc/FT231Driver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   FT231 Driver header
 *******************************************************************************
 * Copyright (c) 2025 Avery Digital Data
 * All rights reserved.
 ******************************************************************************/

#ifndef __FT231DRIVER_H_
#define __FT231DRIVER_H_

/* Includes ------------------------------------------------------------------*/
//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include "GPIODriver.h"
#include "UARTDriver.h"

/* Macros --------------------------------------------------------------------*/
#define FT231_RSTDelay	SYSCLK/1000000
#define FT231_PONDelay	SYSCLK/20
#define	FT231_Buffer	10000

/* Public Variables ----------------------------------------------------------*/
extern	char				Message[];
extern	volatile uint32_t	MessageSize;
extern	volatile uint8_t	UserMessage;
extern	volatile uint8_t	ModeBusMessage;
extern	char				CollMessage[];
extern	volatile uint32_t	CollMessageSize;
extern	volatile uint8_t	CollUserMessage;
extern	volatile uint8_t	CollModBusMessage;

/* Public function prototypes ------------------------------------------------*/
ErrorStatus FT231Init (void);
ErrorStatus FT231Reset (void);
ErrorStatus FT231Answer (char *Message_String);
ErrorStatus FT231AnswerWOS (char *Message_String);
ErrorStatus FT231ReadDMABuffer( void);


/* IRQHander function prototypes ---------------------------------------------*/
void FT231IRQHandler (void);

#endif /* __FT231DRIVER_H_ */

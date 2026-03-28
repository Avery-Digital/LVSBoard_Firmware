/*******************************************************************************
 * @file    Inc/UARTDriver.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   UART driver header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

#ifndef __UARTDRIVER_H_
#define __UARTDRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_usart.h"
#include "LVSConfig.h"

/* Public function prototypes ------------------------------------------------*/
ErrorStatus UARTInit( UARTHandler *UART_Handler );
ErrorStatus UARTDMARxInit( UARTDMAHandler *UARTDMA_Handler);
ErrorStatus UARTDMATxInit( UARTDMAHandler *UARTDMA_Handler);
ErrorStatus UARTRXInterruptEnable( UARTHandler *UART_Handler );
ErrorStatus UARTRXInterruptDisable( UARTHandler *UART_Handler);
ErrorStatus UARTTransmitByte( UARTHandler *UART_Handler, uint8_t UART_TX_Byte);
ErrorStatus UARTTransmitStringWOCRLF( UARTHandler *UART_Handler, char *UART_TX_String);
ErrorStatus UARTTransmitString( UARTHandler *UART_Handler, char *UART_TX_String);
ErrorStatus UARTReceiveByte( UARTHandler *UART_Handler, uint8_t *UART_RX_Byte );
ErrorStatus UARTDMARxPOP( UARTDMAHandler *UART_Handler, uint8_t *Data);
uint8_t UARTDMATxAvail( UARTDMAHandler *UARTDMA_Handler);
ErrorStatus UARTDMATxPush( UARTDMAHandler *UARTDMA_Handler, uint8_t *Data, uint32_t Data_Length);
ErrorStatus UARTDMARxFlush( UARTDMAHandler *UART_Handler);

#ifdef __cplusplus
}
#endif

#endif /* __UARTDRIVER_H_ */

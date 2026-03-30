/*******************************************************************************
 * @file    Inc/CommDriver.h
 * @brief   Communication Driver — USART3 DMA TX/RX with Packet Protocol
 *
 *          Replaces FT231Driver + UARTDriver with an interrupt-driven
 *          DMA approach modeled after the DMF-Motherboard-R1 USART driver.
 *
 *          RX: Circular DMA on DMA1_Stream0 with HT/TC + USART3 IDLE
 *          TX: Normal DMA on DMA2_Stream0 with TC interrupt
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

#ifndef COMM_DRIVER_H
#define COMM_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_cortex.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_usart.h"
#include "PacketProtocol.h"
#include "LVSConfig.h"

/* =========================== Buffer Sizes ================================= */

#define COMM_RX_BUF_SIZE    4096U       /**< Circular DMA RX buffer          */
#define COMM_TX_BUF_SIZE    PKT_TX_BUF_SIZE /**< Worst-case escaped packet   */

/* =========================== Public API =================================== */

/**
 * @brief  Initialise USART3, GPIO (PD8/PD9), DMA streams, and NVIC.
 *         Also performs FT231 hardware reset via PB15.
 */
void CommDriver_Init(void);

/**
 * @brief  Start circular DMA reception with HT/TC/IDLE interrupts.
 *         Call once after CommDriver_Init().
 */
void CommDriver_StartRx(void);

/**
 * @brief  Build a protocol packet and transmit it via DMA.
 * @return true on success, false if TX is busy
 */
bool CommDriver_SendPacket(uint8_t msg1, uint8_t msg2,
                           uint8_t cmd1, uint8_t cmd2,
                           const uint8_t *payload, uint16_t length);

/**
 * @brief  Check if the transmitter is idle and ready for a new packet.
 */
bool CommDriver_TxReady(void);

/* ======================== ISR Callbacks ==================================== */

/**
 * @brief  Process new DMA RX data and feed to parser.
 *         Called from DMA1_Stream0 HT/TC and USART3 IDLE ISRs.
 */
void CommDriver_RxProcessISR(void);

/**
 * @brief  DMA TX complete callback.
 *         Called from DMA2_Stream0 TC ISR.
 */
void CommDriver_TxCompleteISR(void);

/* ======================== Parser Access ==================================== */

/**
 * @brief  Get a pointer to the protocol parser instance.
 *         Used by CommandInterface to register its callback.
 */
ProtocolParser *CommDriver_GetParser(void);

#ifdef __cplusplus
}
#endif

#endif /* COMM_DRIVER_H */

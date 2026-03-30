/*******************************************************************************
 * @file    Src/CommDriver.c
 * @brief   Communication Driver — USART3 DMA TX/RX with Packet Protocol
 *
 *          RX: Circular DMA on DMA1_Stream0 with three interrupt sources:
 *            - DMA Half-Transfer (HT) — buffer 50% full
 *            - DMA Transfer-Complete (TC) — buffer 100% full (wraps)
 *            - USART3 IDLE line — gap after last byte (end of packet)
 *          All three call CommDriver_RxProcessISR() which feeds new bytes
 *          into the protocol parser.
 *
 *          TX: Normal-mode DMA on DMA2_Stream0.  TC interrupt clears busy
 *          flag so the next packet can be queued.
 *
 *          Hardware: USART3 on PD8 (TX) / PD9 (RX), FT231 reset on PB15.
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "CommDriver.h"
#include "GPIODriver.h"
#include <string.h>

/* ==========================================================================
 *  PRIVATE DATA
 * ========================================================================== */

/** RX circular buffer — placed in D2 SRAM for DMA access */
__attribute__((section(".RAM_D2"), aligned(32), used))
static uint8_t rx_buf[COMM_RX_BUF_SIZE];

/** TX buffer — worst-case escaped packet */
static uint8_t tx_buf[COMM_TX_BUF_SIZE];

/** Protocol parser instance */
static ProtocolParser parser;

/** DMA RX read position tracker */
static volatile uint16_t rx_head = 0;

/** TX busy flag */
static volatile bool tx_busy = false;

/* ==========================================================================
 *  FT231 RESET (PB15)
 * ========================================================================== */

static void FT231_HardwareReset(void)
{
    /* Use the existing GPIO reset/set from GPIODriver via the USB_FT231 config */
    GPIOReset(USB_FT231.nRST_Pin);
    Delay(FT231_RSTDelay);
    GPIOSet(USB_FT231.nRST_Pin);
    Delay(FT231_PONDelay);
}

/* ==========================================================================
 *  COMM DRIVER — INIT
 * ========================================================================== */

void CommDriver_Init(void)
{
    /* ---- FT231 reset pin (PB15) ---- */
    GPIOInit(USB_FT231.nRST_Pin);
    FT231_HardwareReset();

    /* ---- GPIO: PD8 (TX) and PD9 (RX) as AF7 for USART3 ---- */
    LL_AHB4_GRP1_EnableClock(LL_AHB4_GRP1_PERIPH_GPIOD);

    LL_GPIO_InitTypeDef gpio = {0};
    gpio.Mode       = LL_GPIO_MODE_ALTERNATE;
    gpio.Speed      = LL_GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    gpio.Pull       = LL_GPIO_PULL_NO;
    gpio.Alternate  = LL_GPIO_AF_7;

    gpio.Pin = LL_GPIO_PIN_8;
    LL_GPIO_Init(GPIOD, &gpio);

    gpio.Pin = LL_GPIO_PIN_9;
    LL_GPIO_Init(GPIOD, &gpio);

    /* ---- USART3 peripheral clock ---- */
    LL_RCC_SetUSARTClockSource(LL_RCC_USART234578_CLKSOURCE_PLL3Q);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

    /* ---- USART3 configuration ---- */
    LL_USART_Disable(USART3);

    LL_USART_SetPrescaler(USART3, LL_USART_PRESCALER_DIV1);
    LL_USART_SetBaudRate(USART3,
                         128000000UL,       /* PLL3Q = 128 MHz */
                         LL_USART_PRESCALER_DIV1,
                         LL_USART_OVERSAMPLING_16,
                         115200);
    LL_USART_SetDataWidth(USART3, LL_USART_DATAWIDTH_8B);
    LL_USART_SetStopBitsLength(USART3, LL_USART_STOPBITS_1);
    LL_USART_SetParity(USART3, LL_USART_PARITY_NONE);
    LL_USART_SetTransferDirection(USART3, LL_USART_DIRECTION_TX_RX);
    LL_USART_SetHWFlowCtrl(USART3, LL_USART_HWCONTROL_NONE);
    LL_USART_SetOverSampling(USART3, LL_USART_OVERSAMPLING_16);

    LL_USART_DisableFIFO(USART3);
    LL_USART_ConfigAsyncMode(USART3);

    /* ---- DMA1 Stream 0 — RX (circular) ---- */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

    LL_DMA_DisableStream(DMA1, LL_DMA_STREAM_0);
    while (LL_DMA_IsEnabledStream(DMA1, LL_DMA_STREAM_0));

    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_STREAM_0, LL_DMAMUX1_REQ_USART3_RX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_STREAM_0, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
    LL_DMA_SetStreamPriorityLevel(DMA1, LL_DMA_STREAM_0, LL_DMA_PRIORITY_MEDIUM);
    LL_DMA_SetMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MODE_CIRCULAR);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_DisableFifoMode(DMA1, LL_DMA_STREAM_0);

    LL_DMA_SetPeriphAddress(DMA1, LL_DMA_STREAM_0,
                            LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_RECEIVE));
    LL_DMA_SetMemoryAddress(DMA1, LL_DMA_STREAM_0, (uint32_t)rx_buf);
    LL_DMA_SetDataLength(DMA1, LL_DMA_STREAM_0, COMM_RX_BUF_SIZE);

    NVIC_SetPriority(DMA1_Stream0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
    NVIC_EnableIRQ(DMA1_Stream0_IRQn);

    /* ---- DMA2 Stream 0 — TX (normal) ---- */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);

    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_0));

    LL_DMA_SetPeriphRequest(DMA2, LL_DMA_STREAM_0, LL_DMAMUX1_REQ_USART3_TX);
    LL_DMA_SetDataTransferDirection(DMA2, LL_DMA_STREAM_0, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetStreamPriorityLevel(DMA2, LL_DMA_STREAM_0, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphSize(DMA2, LL_DMA_STREAM_0, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA2, LL_DMA_STREAM_0, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_SetPeriphIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA2, LL_DMA_STREAM_0, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_DisableFifoMode(DMA2, LL_DMA_STREAM_0);

    LL_DMA_SetPeriphAddress(DMA2, LL_DMA_STREAM_0,
                            LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT));

    NVIC_SetPriority(DMA2_Stream0_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 3, 0));
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);

    /* ---- Enable DMA requests in USART3 ---- */
    LL_USART_EnableDMAReq_TX(USART3);
    LL_USART_EnableDMAReq_RX(USART3);

    /* ---- USART3 IDLE line interrupt ---- */
    LL_USART_EnableIT_IDLE(USART3);

    /* ---- USART3 NVIC ---- */
    NVIC_SetPriority(USART3_IRQn,
                     NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
    NVIC_EnableIRQ(USART3_IRQn);

    /* ---- Enable USART3 ---- */
    LL_USART_Enable(USART3);

    while (!LL_USART_IsActiveFlag_TEACK(USART3));
    while (!LL_USART_IsActiveFlag_REACK(USART3));

    rx_head = 0;
    tx_busy = false;
}

/* ==========================================================================
 *  COMM DRIVER — START RX
 * ========================================================================== */

void CommDriver_StartRx(void)
{
    /* Enable DMA RX interrupts: half-transfer, transfer-complete, error */
    LL_DMA_EnableIT_HT(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_STREAM_0);

    /* Start circular DMA reception */
    LL_DMA_EnableStream(DMA1, LL_DMA_STREAM_0);
}

/* ==========================================================================
 *  COMM DRIVER — RX PROCESS (called from ISR)
 * ========================================================================== */

void CommDriver_RxProcessISR(void)
{
    /* Current DMA write position = buf_size - NDTR */
    uint16_t dma_write_pos = COMM_RX_BUF_SIZE -
        (uint16_t)LL_DMA_GetDataLength(DMA1, LL_DMA_STREAM_0);

    uint16_t read_pos = rx_head;

    if (dma_write_pos == read_pos) {
        return;
    }

    if (dma_write_pos > read_pos) {
        /* Linear — no wrap */
        Protocol_FeedBytes(&parser,
                           &rx_buf[read_pos],
                           dma_write_pos - read_pos);
    } else {
        /* Wrapped — process end of buffer, then beginning */
        Protocol_FeedBytes(&parser,
                           &rx_buf[read_pos],
                           COMM_RX_BUF_SIZE - read_pos);

        if (dma_write_pos > 0) {
            Protocol_FeedBytes(&parser,
                               &rx_buf[0],
                               dma_write_pos);
        }
    }

    rx_head = dma_write_pos;
}

/* ==========================================================================
 *  COMM DRIVER — SEND PACKET
 * ========================================================================== */

bool CommDriver_SendPacket(uint8_t msg1, uint8_t msg2,
                           uint8_t cmd1, uint8_t cmd2,
                           const uint8_t *payload, uint16_t length)
{
    if (tx_busy) {
        return false;
    }

    uint16_t frame_len = Protocol_BuildPacket(
        tx_buf,
        msg1, msg2, cmd1, cmd2,
        payload, length
    );

    if (frame_len > COMM_TX_BUF_SIZE) {
        return false;
    }

    tx_busy = true;

    LL_DMA_DisableStream(DMA2, LL_DMA_STREAM_0);
    while (LL_DMA_IsEnabledStream(DMA2, LL_DMA_STREAM_0));

    LL_DMA_SetMemoryAddress(DMA2, LL_DMA_STREAM_0, (uint32_t)tx_buf);
    LL_DMA_SetDataLength(DMA2, LL_DMA_STREAM_0, frame_len);

    /* Clear pending flags for stream 0 */
    LL_DMA_ClearFlag_TC0(DMA2);
    LL_DMA_ClearFlag_HT0(DMA2);
    LL_DMA_ClearFlag_TE0(DMA2);

    /* Enable TX complete and error interrupts */
    LL_DMA_EnableIT_TC(DMA2, LL_DMA_STREAM_0);
    LL_DMA_EnableIT_TE(DMA2, LL_DMA_STREAM_0);

    LL_DMA_EnableStream(DMA2, LL_DMA_STREAM_0);

    return true;
}

/* ==========================================================================
 *  COMM DRIVER — TX READY CHECK
 * ========================================================================== */

bool CommDriver_TxReady(void)
{
    return !tx_busy;
}

/* ==========================================================================
 *  COMM DRIVER — TX COMPLETE (called from ISR)
 * ========================================================================== */

void CommDriver_TxCompleteISR(void)
{
    tx_busy = false;
}

/* ==========================================================================
 *  COMM DRIVER — PARSER ACCESS
 * ========================================================================== */

ProtocolParser *CommDriver_GetParser(void)
{
    return &parser;
}

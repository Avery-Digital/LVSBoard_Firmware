/*******************************************************************************
 * @file    Src/UARTDriver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   UART Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "UARTDriver.h"

/* Private Variables ---------------------------------------------------------*/
static volatile uint16_t RX_Tail = 0;
#define RING_SZ		32768u  // 32 KB (<= 65535 NDTR limit)
#define RING_MASK	(RING_SZ - 1)
__attribute__((section(".RAM_D2"), aligned(32), used)) static uint8_t RxRing[RING_SZ];

/* Public functions ----------------------------------------------------------*/
ErrorStatus UARTInit( UARTHandler *UART_Handler)
{
	ErrorStatus Status = SUCCESS;

	/* Enable clock for UART's GPIO */
	LL_AHB4_GRP1_EnableClock( UART_Handler->UART_GPIO0_Clk);

	/* Configure UART's GPIO */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin				= UART_Handler->UART_GPIO0_Pin;
    GPIO_InitStruct.Mode			= LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed			= LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType		= LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull			= LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   	= UART_Handler->UART_GPIO0_AF;
    LL_GPIO_Init ( UART_Handler->UART_GPIO0_Port, &GPIO_InitStruct);

    if ( UART_Handler->UART_CMPLX_Port == TRUE)
    {
    	/* Enable clock for I2C's GPIO1 */
    	LL_AHB4_GRP1_EnableClock( UART_Handler->UART_GPIO1_Clk);

    	/* Configure I2C's GPIO1 */
    	GPIO_InitStruct.Pin			= UART_Handler->UART_GPIO1_Pin;
        GPIO_InitStruct.Alternate   = UART_Handler->UART_GPIO1_AF;
    	LL_GPIO_Init ( UART_Handler->UART_GPIO1_Port, &GPIO_InitStruct);
    }

	/* Enable clock for UART */
    LL_RCC_SetUSARTClockSource(UART_Handler->UART_Clk_Source);
    /* Enable peripheral bus */
    switch( UART_Handler->UART_Bus)
    {
    case APB1:
    	LL_APB1_GRP1_EnableClock( UART_Handler->UART_Clk);
    	break;
    case APB2:
    	LL_APB2_GRP1_EnableClock( UART_Handler->UART_Clk);
    	break;
    case APB3:
    	LL_APB3_GRP1_EnableClock( UART_Handler->UART_Clk);
    	break;
    case APB4:
    	LL_APB4_GRP1_EnableClock( UART_Handler->UART_Clk);
    	break;
    default:
    	Status += ERROR;
    	break;
    }

    /* Configure UART */
    LL_USART_InitTypeDef UART_InitStruct;
    UART_InitStruct.PrescalerValue		= UART_Handler->UART_Clk_PrescalerValue;
    UART_InitStruct.BaudRate			= UART_Handler->UART_Baudrate;
	UART_InitStruct.DataWidth			= LL_USART_DATAWIDTH_8B;
	UART_InitStruct.StopBits			= LL_USART_STOPBITS_1;
	UART_InitStruct.Parity				= LL_USART_PARITY_NONE;
	UART_InitStruct.TransferDirection	= UART_Handler->UART_TransferDirection;
	UART_InitStruct.HardwareFlowControl = UART_Handler->UART_FlowControl;
	UART_InitStruct.OverSampling		= LL_USART_OVERSAMPLING_16;
	LL_USART_Init (UART_Handler->UART_Peripheral, &UART_InitStruct);

	/* Asynchronous mode without FIFO */
	LL_USART_DisableFIFO( UART_Handler->UART_Peripheral);
	LL_USART_ConfigAsyncMode( UART_Handler->UART_Peripheral);

	/* Enable UART */
	LL_USART_Enable(UART_Handler->UART_Peripheral);

    return Status;
}

ErrorStatus UARTDMARxInit( UARTDMAHandler *UARTDMA_Handler)
{
	/* Enable clock for DMA that UART is connected to */
	LL_AHB1_GRP1_EnableClock( UARTDMA_Handler->UART_DMA_Clk);

    /* Configure DMA for UART */
    LL_DMA_InitTypeDef DMA_InitStruct = {0};
    DMA_InitStruct.PeriphRequest			= UARTDMA_Handler->UART_DMA_PeriphRequest;
    DMA_InitStruct.Direction				= UARTDMA_Handler->UART_DMA_Direction;
    DMA_InitStruct.Mode						= UARTDMA_Handler->UART_DMA_Mode;
    DMA_InitStruct.PeriphOrM2MSrcAddress	= LL_USART_DMA_GetRegAddr(UARTDMA_Handler->UART_Peripheral, UARTDMA_Handler->UART_Direction);
    DMA_InitStruct.MemoryOrM2MDstAddress	= (uint32_t) &RxRing[0];
    DMA_InitStruct.PeriphOrM2MSrcIncMode	= LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.MemoryOrM2MDstIncMode	= LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.PeriphOrM2MSrcDataSize	= LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstDataSize	= LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.NbData					= (uint32_t) RING_SZ;
    DMA_InitStruct.Priority					= UARTDMA_Handler->UART_DMA_Priority;
    DMA_InitStruct.FIFOMode					= LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream, &DMA_InitStruct);

    /* Enable UART DMA Rx */
   	LL_USART_EnableDMAReq_RX( UARTDMA_Handler->UART_Peripheral);

    /* Enable stream */
    LL_DMA_EnableStream( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream);

    return SUCCESS;
}

ErrorStatus UARTDMATxInit( UARTDMAHandler *UARTDMA_Handler)
{
	/* Enable clock for DMA that UART is connected to */
	LL_AHB1_GRP1_EnableClock(UARTDMA_Handler->UART_DMA_Clk);

    /* Configure DMA for UART */
    LL_DMA_InitTypeDef DMA_InitStruct = {0};
    DMA_InitStruct.PeriphRequest			= UARTDMA_Handler->UART_DMA_PeriphRequest;
    DMA_InitStruct.Direction				= UARTDMA_Handler->UART_DMA_Direction;
    DMA_InitStruct.Mode						= UARTDMA_Handler->UART_DMA_Mode;
    DMA_InitStruct.PeriphOrM2MSrcAddress	= LL_USART_DMA_GetRegAddr(UARTDMA_Handler->UART_Peripheral, UARTDMA_Handler->UART_Direction);
    DMA_InitStruct.MemoryOrM2MDstAddress	= 0;
    DMA_InitStruct.PeriphOrM2MSrcIncMode	= LL_DMA_PERIPH_NOINCREMENT;
    DMA_InitStruct.MemoryOrM2MDstIncMode	= LL_DMA_MEMORY_INCREMENT;
    DMA_InitStruct.PeriphOrM2MSrcDataSize	= LL_DMA_PDATAALIGN_BYTE;
    DMA_InitStruct.MemoryOrM2MDstDataSize	= LL_DMA_MDATAALIGN_BYTE;
    DMA_InitStruct.NbData					= 0;
    DMA_InitStruct.Priority					= UARTDMA_Handler->UART_DMA_Priority;
    DMA_InitStruct.FIFOMode					= LL_DMA_FIFOMODE_DISABLE;
    LL_DMA_Init( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream, &DMA_InitStruct);

    /* Enable UART DMA Tx */
   	LL_USART_EnableDMAReq_TX( UARTDMA_Handler->UART_Peripheral);

    return SUCCESS;
}

ErrorStatus UARTRxInterruptEnable( UARTHandler *UART_Handler)
{
	/* Configure NVIC */
	NVIC_EnableIRQ( UART_Handler->UART_IRQn);
	NVIC_SetPriority( UART_Handler->UART_IRQn, UART_Handler->UART_IRQ_Priority);

	/* Enable interrupt for data arrival */
	LL_USART_EnableIT_RXNE( UART_Handler->UART_Peripheral);
	return (SUCCESS);
}

ErrorStatus UARTRxInterruptDisable( UARTHandler *UART_Handler)
{
	/* Configure NVIC */
	NVIC_DisableIRQ( UART_Handler->UART_IRQn);

	/* Disable interrupt for data arrival */
	LL_USART_DisableIT_RXNE( UART_Handler->UART_Peripheral);
	return (SUCCESS);
}

ErrorStatus UARTTransmitByte( UARTHandler *UART_Handler, uint8_t UART_TX_Byte)
{
	/* Wait for TX register ready flag */
	while (!LL_USART_IsActiveFlag_TXE( UART_Handler->UART_Peripheral));

	/* Put the data in TX register */
	LL_USART_TransmitData8( UART_Handler->UART_Peripheral, UART_TX_Byte);

	/* Wait for TX register clear flag */
	while (!LL_USART_IsActiveFlag_TC( UART_Handler->UART_Peripheral));

    return (SUCCESS);
}

ErrorStatus UARTTransmitStringWOCRLF( UARTHandler *UART_Handler, char *UART_TX_String)
{
	uint32_t i = 0;
	while (UART_TX_String[i] != 0x0)
	{
		/* Transmit i_th Byte */
		UARTTransmitByte( UART_Handler, UART_TX_String[i]);

		/* increase i */
		i++;
	}

    return (SUCCESS);
}

ErrorStatus UARTTransmitString( UARTHandler *UART_Handler, char *UART_TX_String)
{
	uint32_t i = 0;
	while (UART_TX_String[i] != 0x0)
	{
		/* Transmit i_th Byte */
		UARTTransmitByte( UART_Handler, UART_TX_String[i]);

		/* increase i */
		i++;
	}

	/* Transmit carriage return character */
	UARTTransmitByte( UART_Handler, '\r');

	/* Transmit  newline character */
	UARTTransmitByte( UART_Handler, '\n');

	/* Wait for TX register clear flag */
	while (!LL_USART_IsActiveFlag_TC(UART_Handler->UART_Peripheral));

    return SUCCESS;
}

ErrorStatus UARTReceiveByte( UARTHandler *UART_Handler, uint8_t *UART_RX_Byte )
{
	if (LL_USART_IsActiveFlag_ORE( UART_Handler->UART_Peripheral))
	{
		LL_USART_ClearFlag_ORE( UART_Handler->UART_Peripheral);
		LL_USART_ReceiveData8( UART_Handler->UART_Peripheral);
		return (ERROR);
	}
	else
	{
		/* Wait for RX register ready flag */
		while (!LL_USART_IsActiveFlag_RXNE( UART_Handler->UART_Peripheral));
		*UART_RX_Byte = LL_USART_ReceiveData8( UART_Handler->UART_Peripheral);
	}
    return SUCCESS;
}

ErrorStatus UARTDMARxPOP( UARTDMAHandler *UARTDMA_Handler, uint8_t *Data)
{
	ErrorStatus Status = SUCCESS;
	uint16_t RX_Head;

	RX_Head = RING_SZ - ((uint16_t) LL_DMA_GetDataLength( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream));
	RX_Head = RX_Head & RING_MASK;

	if (RX_Head == RX_Tail)
	{
		Status += ERROR;
	}
	else
	{
		*Data = RxRing[RX_Tail];
		RX_Tail = (RX_Tail + 1) & RING_MASK;
	}

	return Status;
}

uint8_t UARTDMATxAvail( UARTDMAHandler *UARTDMA_Handler)
{
	uint8_t UARTDMATxIsAvail = 0;

	/* Check to see if data transmission is possible */
	if (LL_DMA_IsEnabledStream( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream) == 0)
		UARTDMATxIsAvail = 1;
	else
		UARTDMATxIsAvail = 0;

	return UARTDMATxIsAvail;
}

ErrorStatus UARTDMATxPush( UARTDMAHandler *UARTDMA_Handler, uint8_t *Data, uint32_t Data_Length)
{
	ErrorStatus Status = SUCCESS;


	/* Check to see if data transmission is possible */
	if (LL_DMA_IsEnabledStream( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream))
		Status += ERROR;

    if (Status == SUCCESS)
    {
    	/* Wait for previous transaction to finish */
    	while(!LL_USART_IsActiveFlag_TC( UARTDMA_Handler->UART_Peripheral));

    	/* Configure the new transmission */
    	LL_DMA_SetMemoryAddress( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream, (uint32_t) Data);
    	LL_DMA_SetDataLength( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream, Data_Length);

    	/* Clear TC flag */
    	/* STM32H7Cube LL Driver Bug
    	 * We need function "LL_DMA_ClearFlag_TC" which is not provided.
    	 */
    	if (UARTDMA_Handler->UART_DMA_Stream == LL_DMA_STREAM_0)
    		LL_DMA_ClearFlag_TC0( UARTDMA_Handler->UART_DMA_Peripheral);

    	/* Start data transmission */
    	LL_USART_ClearFlag_TC( UARTDMA_Handler->UART_Peripheral);

        /* Enable stream */
    	LL_DMA_EnableStream( UARTDMA_Handler->UART_DMA_Peripheral, UARTDMA_Handler->UART_DMA_Stream);
    }

	return Status;
}

ErrorStatus UARTDMARxFlush( UARTDMAHandler *UART_Handler)
{
	uint16_t RX_Head;

	RX_Head = RING_SZ - ((uint16_t) LL_DMA_GetDataLength( UART_Handler->UART_DMA_Peripheral, UART_Handler->UART_DMA_Stream));
	RX_Tail = RX_Head & RING_MASK;

	return SUCCESS;
}


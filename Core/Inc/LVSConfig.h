/*******************************************************************************
 * @file    Inc/LVSConfig.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Low Voltage Switching Board Configuration Header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

#ifndef __LVSCONFIG_H_
#define __LVSCONFIG_H_

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdint.h>
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_dma.h"
#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_i2c.h"
#include "stm32h7xx_ll_rcc.h"
#include "stm32h7xx_ll_spi.h"
#include "stm32h7xx_ll_usart.h"
#include "stm32h7xx_ll_cortex.h"

/* Function ------------------------------------------------------------------*/
#define	Delay(Cycles)	for (uint32_t Counts=0; Counts<Cycles; Counts++) __NOP()

/* Enumerated ----------------------------------------------------------------*/
typedef enum {APB1, APB2, APB3, APB4} PeriphBus;
typedef enum {GRP1, GRP2} PeriphGroup;

typedef enum
{
  FALSE	= 0,
  TRUE	= !FALSE
} condition;

/* Types ---------------------------------------------------------------------*/
typedef struct
{
	uint32_t			GPIO_Clk;
	GPIO_TypeDef	   *GPIO_Port;
	uint32_t 			GPIO_Pin;
	uint32_t			GPIO_Mode;
	uint32_t			GPIO_Speed;
	uint32_t			GPIO_Pull;
} GPIOHandler;

typedef struct
{
	condition			I2C_CMPLX_Port;
	uint32_t			I2C_GPIO0_Clk;
	GPIO_TypeDef	   *I2C_GPIO0_Port;
	uint32_t 			I2C_GPIO0_Pin;
	uint32_t			I2C_GPIO0_AF;
	uint32_t			I2C_GPIO1_Clk;
	GPIO_TypeDef	   *I2C_GPIO1_Port;
	uint32_t 			I2C_GPIO1_Pin;
	uint32_t			I2C_GPIO1_AF;
	PeriphBus			I2C_Bus;
	uint32_t			I2C_Clk;
	uint32_t			I2C_Clk_Source;
	uint32_t			I2C_Clk_PrescalerValue;
	I2C_TypeDef		   *I2C_Peripheral;
	uint32_t			I2C_Timing;
	uint32_t			I2C_AnalogFilter;
	uint32_t			I2C_DigitalFilter;
	uint32_t			I2C_Achnowledgement;
	uint32_t			I2C_Addressing_Mode;
} I2CHandler;

typedef struct
{
	condition			SPI_CMPLX_Port;
	uint32_t			SPI_GPIO0_Clk;
	GPIO_TypeDef	   *SPI_GPIO0_Port;
	uint32_t	 		SPI_GPIO0_Pin;
	uint32_t			SPI_GPIO0_AF;
	uint32_t			SPI_GPIO1_Clk;
	GPIO_TypeDef	   *SPI_GPIO1_Port;
	uint32_t 			SPI_GPIO1_Pin;
	uint32_t			SPI_GPIO1_AF;
	uint32_t			SPI_GPIO2_Clk;
	GPIO_TypeDef	   *SPI_GPIO2_Port;
	uint32_t 			SPI_GPIO2_Pin;
	uint32_t			SPI_GPIO2_AF;
	PeriphBus			SPI_Bus;
	uint32_t			SPI_Clk;
	uint32_t			SPI_Clk_Source;
	SPI_TypeDef		   *SPI_Peripheral;
	uint32_t			SPI_TransferDirection;
	uint32_t			SPI_DataWidth;
	uint32_t			SPI_ClockPolarity;
	uint32_t			SPI_ClockPhase;
	uint32_t			SPI_BaudRate;
	uint32_t			SPI_BitOrder;
} SPIHandler;

typedef struct
{
	uint32_t			UART_DMA_Clk;
	DMA_TypeDef			*UART_DMA_Peripheral;
	uint32_t 			UART_DMA_PeriphRequest;
	uint32_t 			UART_DMA_Stream;
	uint32_t			UART_DMA_Direction;
	uint32_t 			UART_DMA_Mode;
	uint32_t 			UART_DMA_Priority;
	IRQn_Type 			UART_DMA_IRQn;
	uint32_t 			UART_DMA_IRQ_Priority;
	USART_TypeDef		*UART_Peripheral;
	uint32_t			UART_Direction;
} UARTDMAHandler;

typedef struct
{
	condition			UART_CMPLX_Port;
	uint32_t			UART_GPIO0_Clk;
	GPIO_TypeDef	   *UART_GPIO0_Port;
	uint32_t 			UART_GPIO0_Pin;
	uint32_t			UART_GPIO0_AF;
	uint32_t			UART_GPIO1_Clk;
	GPIO_TypeDef	   *UART_GPIO1_Port;
	uint32_t 			UART_GPIO1_Pin;
	uint32_t			UART_GPIO1_AF;
	PeriphBus			UART_Bus;
	uint32_t			UART_Clk;
	uint32_t			UART_Clk_Source;
	uint32_t			UART_Clk_PrescalerValue;
	USART_TypeDef	   *UART_Peripheral;
	uint32_t 			UART_Baudrate;
	uint32_t 			UART_TransferDirection;
	uint32_t			UART_FlowControl;
	IRQn_Type 			UART_IRQn;
	uint32_t 			UART_IRQ_Priority;
} UARTHandler;

typedef struct
{
	I2CHandler		   *B0I2C_Interface;
	SPIHandler		   *B0SPI_Interface;
	GPIOHandler		   *B0nCS_Pin;
	GPIOHandler		   *B0nRST_Pin;
	I2CHandler		   *B1I2C_Interface;
	SPIHandler		   *B1SPI_Interface;
	GPIOHandler		   *B1nCS_Pin;
	GPIOHandler		   *B1nRST_Pin;
} EVSHandler;

typedef struct
{
	UARTHandler		   *UART_Interface;
	UARTDMAHandler	   *UART_DMATx;
	UARTDMAHandler	   *UART_DMARx;
	GPIOHandler		   *nRST_Pin;
} USBHandler;
/* BIST Configuration --------------------------------------------------------*/
extern	uint8_t					BISTPass;

/* MCU Configuration ---------------------------------------------------------*/
#define	HSEFreq				 16000000UL
#define PLL1InFreq			  2000000UL
#define	PLL1OutFreq			512000000UL
#define	SYSCLK				512000000UL
#define PLL1M				8U					// HSEFreq/PLL1InFreq
#define	PLL1N				256U				// PLL1OutFreq/PLL1InFreq
#define	PLL1PFreq			512000000UL
#define	PLL1QFreq			256000000UL
#define	PLL1RFreq			256000000UL
#define PLL1P				1U					// PLL1OutFreq/SYSCLK
#define	PLL1Q				2U					// PLL1OutFreq/PLL1QFreq
#define	PLL1R				2U					// PLL1OutFreq/PLL1RFreq

/* Status Manager Configuration ----------------------------------------------*/
extern	GPIOHandler 		LED_Status_VSB0;
extern	GPIOHandler 		LED_Status_VSB1;

/* Voltage Switcher Configuration --------------------------------------------*/
#define	EVSADG729CH0Addr	0x44
#define	EVSADG729CH1Addr	0x45
#define	EVSADG729CH2Addr	0x47
extern	EVSHandler			VS_ADG714;

/* FT231 Timing Constants ----------------------------------------------------*/
#define FT231_RSTDelay	SYSCLK/1000000
#define FT231_PONDelay	SYSCLK/20

/* USB Configuration ---------------------------------------------------------*/
extern USBHandler 			USB_FT231;

#endif /* __LVSCONFIG_H_ */

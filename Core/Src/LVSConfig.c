/*******************************************************************************
 * @file    Src/LVSConfig.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Low Voltage Switching Board Configuration
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "LVSConfig.h"

/* Board Configuration Variables ---------------------------------------------*/
uint8_t BISTPass = 0;

/* Status Manager Configuration ----------------------------------------------*/
GPIOHandler 	LED_Status_VSB0 = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOF,
	.GPIO_Port						= GPIOF,
	.GPIO_Pin						= LL_GPIO_PIN_14,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* LED_Status_VSB0 */

GPIOHandler 	LED_Status_VSB1 = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOF,
	.GPIO_Port						= GPIOF,
	.GPIO_Pin						= LL_GPIO_PIN_15,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* LED_Status_VSB1 */

/* Voltage Switcher Configuration --------------------------------------------*/
I2CHandler		VSB0_ADG729_I2C		= {
	.I2C_CMPLX_Port					= FALSE,
	.I2C_GPIO0_Clk					= LL_AHB4_GRP1_PERIPH_GPIOB,
	.I2C_GPIO0_Port					= GPIOB,
	.I2C_GPIO0_Pin					= LL_GPIO_PIN_6|LL_GPIO_PIN_7,
	.I2C_GPIO0_AF					= LL_GPIO_AF_4,
	.I2C_GPIO1_Clk					= LL_AHB4_GRP1_PERIPH_GPIOB,
	.I2C_GPIO1_Port					= GPIOB,
	.I2C_GPIO1_Pin					= LL_GPIO_PIN_6|LL_GPIO_PIN_7,
	.I2C_GPIO1_AF					= LL_GPIO_AF_4,
	.I2C_Bus						= APB1,
	.I2C_Clk						= LL_APB1_GRP1_PERIPH_I2C1,
	.I2C_Clk_Source					= LL_RCC_I2C123_CLKSOURCE_PLL3R,
	.I2C_Peripheral					= I2C1,
	.I2C_Timing						= 0x2040184B,								// For 400kHz bus. Use CubeMX to generate it
	.I2C_AnalogFilter				= LL_I2C_ANALOGFILTER_ENABLE,
	.I2C_DigitalFilter				= 0x0,
	.I2C_Achnowledgement			= LL_I2C_ACK,
	.I2C_Addressing_Mode			= LL_I2C_ADDRESSING_MODE_7BIT,
}; /* VSB0_ADG729_I2C */

SPIHandler		VSB0_ADG714_SPI		= {
	.SPI_CMPLX_Port					= FALSE,
	.SPI_GPIO0_Clk					= LL_AHB4_GRP1_PERIPH_GPIOC,
	.SPI_GPIO0_Port					= GPIOC,
	.SPI_GPIO0_Pin					= LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12,
	.SPI_GPIO0_AF					= LL_GPIO_AF_6,
	.SPI_GPIO1_Clk					= LL_AHB4_GRP1_PERIPH_GPIOC,
	.SPI_GPIO1_Port					= GPIOC,
	.SPI_GPIO1_Pin					= LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12,
	.SPI_GPIO1_AF					= LL_GPIO_AF_6,
	.SPI_GPIO2_Clk					= LL_AHB4_GRP1_PERIPH_GPIOC,
	.SPI_GPIO2_Port					= GPIOC,
	.SPI_GPIO2_Pin					= LL_GPIO_PIN_10|LL_GPIO_PIN_11|LL_GPIO_PIN_12,
	.SPI_GPIO2_AF					= LL_GPIO_AF_6,
	.SPI_Bus						= APB1,
	.SPI_Clk						= LL_APB1_GRP1_PERIPH_SPI3,
	.SPI_Clk_Source					= LL_RCC_SPI123_CLKSOURCE_PLL3P,			// 128MHz -> Look into main.c
	.SPI_Peripheral					= SPI3,
	.SPI_TransferDirection          = LL_SPI_FULL_DUPLEX,
	.SPI_DataWidth					= LL_SPI_DATAWIDTH_16BIT,
	.SPI_ClockPolarity				= LL_SPI_POLARITY_HIGH,
	.SPI_ClockPhase					= LL_SPI_PHASE_2EDGE,
	.SPI_BaudRate                   = LL_SPI_BAUDRATEPRESCALER_DIV256,			// 500Kbps -> Hardware limits
	.SPI_BitOrder					= LL_SPI_MSB_FIRST,
}; /* VSB0_ADG714_SPI */

GPIOHandler 	VSB0_ADG714_nCS = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOD,
	.GPIO_Port						= GPIOD,
	.GPIO_Pin						= LL_GPIO_PIN_0,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* VSB0_ADG714_nCS */

GPIOHandler 	VSB0_ADG714_nRST = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOD,
	.GPIO_Port						= GPIOD,
	.GPIO_Pin						= LL_GPIO_PIN_1,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* VSB0_ADG714_nRST */

I2CHandler		VSB1_ADG729_I2C		= {
	.I2C_CMPLX_Port					= FALSE,
	.I2C_GPIO0_Clk					= LL_AHB4_GRP1_PERIPH_GPIOD,
	.I2C_GPIO0_Port					= GPIOD,
	.I2C_GPIO0_Pin					= LL_GPIO_PIN_12|LL_GPIO_PIN_13,
	.I2C_GPIO0_AF					= LL_GPIO_AF_4,
	.I2C_GPIO1_Clk					= LL_AHB4_GRP1_PERIPH_GPIOD,
	.I2C_GPIO1_Port					= GPIOD,
	.I2C_GPIO1_Pin					= LL_GPIO_PIN_12|LL_GPIO_PIN_13,
	.I2C_GPIO1_AF					= LL_GPIO_AF_4,
	.I2C_Bus						= APB4,
	.I2C_Clk						= LL_APB4_GRP1_PERIPH_I2C4,
	.I2C_Clk_Source					= LL_RCC_I2C4_CLKSOURCE_PLL3R,
	.I2C_Peripheral					= I2C4,
	.I2C_Timing						= 0x2040184B,								// For 400kHz bus. Use CubeMX to generate it
	.I2C_AnalogFilter				= LL_I2C_ANALOGFILTER_ENABLE,
	.I2C_DigitalFilter				= 0x0,
	.I2C_Achnowledgement			= LL_I2C_ACK,
	.I2C_Addressing_Mode			= LL_I2C_ADDRESSING_MODE_7BIT,
}; /* VSB1_ADG729_I2C */

SPIHandler		VSB1_ADG714_SPI		= {
	.SPI_CMPLX_Port					= FALSE,
	.SPI_GPIO0_Clk					= LL_AHB4_GRP1_PERIPH_GPIOA,
	.SPI_GPIO0_Port					= GPIOA,
	.SPI_GPIO0_Pin					= LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7,
	.SPI_GPIO0_AF					= LL_GPIO_AF_5,
	.SPI_GPIO1_Clk					= LL_AHB4_GRP1_PERIPH_GPIOA,
	.SPI_GPIO1_Port					= GPIOA,
	.SPI_GPIO1_Pin					= LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7,
	.SPI_GPIO1_AF					= LL_GPIO_AF_5,
	.SPI_GPIO2_Clk					= LL_AHB4_GRP1_PERIPH_GPIOA,
	.SPI_GPIO2_Port					= GPIOA,
	.SPI_GPIO2_Pin					= LL_GPIO_PIN_5|LL_GPIO_PIN_6|LL_GPIO_PIN_7,
	.SPI_GPIO2_AF					= LL_GPIO_AF_5,
	.SPI_Bus						= APB2,
	.SPI_Clk						= LL_APB2_GRP1_PERIPH_SPI1,
	.SPI_Clk_Source					= LL_RCC_SPI123_CLKSOURCE_PLL3P,			// 128MHz -> Look into main.c
	.SPI_Peripheral					= SPI1,
	.SPI_TransferDirection          = LL_SPI_FULL_DUPLEX,
	.SPI_DataWidth					= LL_SPI_DATAWIDTH_16BIT,
	.SPI_ClockPolarity				= LL_SPI_POLARITY_HIGH,
	.SPI_ClockPhase					= LL_SPI_PHASE_2EDGE,
	.SPI_BaudRate                   = LL_SPI_BAUDRATEPRESCALER_DIV256,			// 500Kbps -> Hardware limits
	.SPI_BitOrder					= LL_SPI_MSB_FIRST,
}; /* VSB1_ADG714_SPI */

GPIOHandler 	VSB1_ADG714_nCS = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOC,
	.GPIO_Port						= GPIOC,
	.GPIO_Pin						= LL_GPIO_PIN_4,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* VSB1_ADG714_nCS */

GPIOHandler 	VSB1_ADG714_nRST = {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOC,
	.GPIO_Port						= GPIOC,
	.GPIO_Pin						= LL_GPIO_PIN_5,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* VSB1_ADG714_nRST */

EVSHandler		VS_ADG714			= {
	.B0I2C_Interface				= &VSB0_ADG729_I2C,
	.B0SPI_Interface				= &VSB0_ADG714_SPI,
	.B0nCS_Pin						= &VSB0_ADG714_nCS,
	.B0nRST_Pin						= &VSB0_ADG714_nRST,
	.B1I2C_Interface				= &VSB1_ADG729_I2C,
	.B1SPI_Interface				= &VSB1_ADG714_SPI,
	.B1nCS_Pin						= &VSB1_ADG714_nCS,
	.B1nRST_Pin						= &VSB1_ADG714_nRST,
}; /* VS_ADG714 */

/* USB Configuration ---------------------------------------------------------*/
UARTHandler USB_FT231_UART = {
	.UART_CMPLX_Port				= FALSE,
	.UART_GPIO0_Clk					= LL_AHB4_GRP1_PERIPH_GPIOD,
	.UART_GPIO0_Port				= GPIOD,
	.UART_GPIO0_Pin					= LL_GPIO_PIN_8|LL_GPIO_PIN_9,
	.UART_GPIO0_AF					= LL_GPIO_AF_7,
	.UART_Bus						= APB1,
	.UART_Clk						= LL_APB1_GRP1_PERIPH_USART3,
	.UART_Clk_Source				= LL_RCC_USART234578_CLKSOURCE_PLL3Q,
	.UART_Clk_PrescalerValue		= LL_USART_PRESCALER_DIV1,
	.UART_Peripheral				= USART3,
	.UART_Baudrate					= 115200,
	.UART_TransferDirection			= LL_USART_DIRECTION_TX_RX,
	.UART_FlowControl				= LL_USART_HWCONTROL_NONE,
	.UART_IRQn						= USART3_IRQn,
	.UART_IRQ_Priority				= 2,
}; /* USB_FT231_UART */

UARTDMAHandler USB_FT231_UARTDMATx	= {
	.UART_DMA_Clk					= LL_AHB1_GRP1_PERIPH_DMA2,
	.UART_DMA_Peripheral			= DMA2,
	.UART_DMA_PeriphRequest			= LL_DMAMUX1_REQ_USART3_TX,
	.UART_DMA_Stream				= LL_DMA_STREAM_0,
	.UART_DMA_Direction				= LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
	.UART_DMA_Mode					= LL_DMA_MODE_NORMAL,
	.UART_DMA_Priority				= LL_DMA_PRIORITY_LOW,
	.UART_Peripheral				= USART3,
	.UART_Direction					= LL_USART_DMA_REG_DATA_TRANSMIT,
}; /* USB_FT231_DMATx */

UARTDMAHandler USB_FT231_UARTDMARx	= {
	.UART_DMA_Clk					= LL_AHB1_GRP1_PERIPH_DMA1,
	.UART_DMA_Peripheral			= DMA1,
	.UART_DMA_PeriphRequest			= LL_DMAMUX1_REQ_USART3_RX,
	.UART_DMA_Stream				= LL_DMA_STREAM_0,
	.UART_DMA_Direction				= LL_DMA_DIRECTION_PERIPH_TO_MEMORY,
	.UART_DMA_Mode					= LL_DMA_MODE_CIRCULAR,
	.UART_DMA_Priority				= LL_DMA_PRIORITY_MEDIUM,
	.UART_Peripheral				= USART3,
	.UART_Direction					= LL_USART_DMA_REG_DATA_RECEIVE,
}; /* USB_FT231_DMARx */

GPIOHandler 	USB_FT231_nRST		= {
	.GPIO_Clk						= LL_AHB4_GRP1_PERIPH_GPIOB,
	.GPIO_Port						= GPIOB,
	.GPIO_Pin						= LL_GPIO_PIN_15,
	.GPIO_Mode						= LL_GPIO_MODE_OUTPUT,
	.GPIO_Speed						= LL_GPIO_SPEED_FREQ_VERY_HIGH,
	.GPIO_Pull						= LL_GPIO_PULL_NO,
}; /* USB_FT231_nRST */

USBHandler		USB_FT231			= {
	.UART_Interface					= &USB_FT231_UART,
	.UART_DMATx						= &USB_FT231_UARTDMATx,
	.UART_DMARx						= &USB_FT231_UARTDMARx,
	.nRST_Pin						= &USB_FT231_nRST,
}; /* USB_FT231 */


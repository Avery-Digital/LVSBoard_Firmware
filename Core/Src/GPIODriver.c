/*******************************************************************************
 * @file    Src/GPIODriver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   GPIO Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "GPIODriver.h"

/* Public functions ----------------------------------------------------------*/
ErrorStatus GPIOInit(GPIOHandler *GPIO_Handler)
{
	/* Enable clock for GPIO */
	LL_AHB4_GRP1_EnableClock(GPIO_Handler->GPIO_Clk);

	/* Configure GPIO */
	LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin			= GPIO_Handler->GPIO_Pin;
	GPIO_InitStruct.Mode		= GPIO_Handler->GPIO_Mode;
	GPIO_InitStruct.Speed		= GPIO_Handler->GPIO_Speed;
	GPIO_InitStruct.OutputType	= LL_GPIO_OUTPUT_PUSHPULL;
	GPIO_InitStruct.Pull		= GPIO_Handler->GPIO_Pull;
	LL_GPIO_Init(GPIO_Handler->GPIO_Port, &GPIO_InitStruct);

    return (SUCCESS);
}

ErrorStatus GPIOReset(GPIOHandler *GPIO_Handler)
{
	LL_GPIO_ResetOutputPin(GPIO_Handler->GPIO_Port, GPIO_Handler->GPIO_Pin);
	return (SUCCESS);
}

ErrorStatus GPIOSet(GPIOHandler *GPIO_Handler)
{
	LL_GPIO_SetOutputPin(GPIO_Handler->GPIO_Port, GPIO_Handler->GPIO_Pin);
	return (SUCCESS);
}

ErrorStatus GPIOToggle(GPIOHandler *GPIO_Handler)
{
	LL_GPIO_TogglePin(GPIO_Handler->GPIO_Port, GPIO_Handler->GPIO_Pin);
	return (SUCCESS);
}

uint8_t GPIORead(GPIOHandler *GPIO_Handler)
{
	uint32_t Temp;

	/* If the port is defined output, read output register */
	if (GPIO_Handler->GPIO_Mode == LL_GPIO_MODE_OUTPUT)
		Temp = LL_GPIO_IsOutputPinSet( GPIO_Handler->GPIO_Port, GPIO_Handler->GPIO_Pin);
	/* Else read input register */
	else
		Temp = LL_GPIO_IsInputPinSet( GPIO_Handler->GPIO_Port, GPIO_Handler->GPIO_Pin);

	/* Return 1 if it's set */
	return (Temp ? 1 : 0);
}

ErrorStatus GPIOReadPort(GPIOHandler *GPIO_Handler, uint16_t* Data)
{
	uint16_t Temp;

	/* If the port is defined output, read output register */
	if (GPIO_Handler->GPIO_Mode == LL_GPIO_MODE_OUTPUT)
		Temp = (uint16_t) LL_GPIO_ReadOutputPort(GPIO_Handler->GPIO_Port);
	/* Else read input register */
	else
		Temp = (uint16_t) LL_GPIO_ReadInputPort(GPIO_Handler->GPIO_Port);

	/* Prepare data before returning back */
	*Data = Temp & GPIO_Handler->GPIO_Pin;

	return (SUCCESS);
}

ErrorStatus GPIOWritePort(GPIOHandler *GPIO_Handler, uint16_t Data)
{
	uint16_t Temp;

	/* Read port content */
	Temp = (uint16_t) LL_GPIO_ReadOutputPort(GPIO_Handler->GPIO_Port);

	/* Mask data */
	Data = Data & (GPIO_Handler->GPIO_Pin);
	Temp = Temp & (~(GPIO_Handler->GPIO_Pin));
	Data = Temp | Data;

	/* Write port content */
	LL_GPIO_WriteOutputPort(GPIO_Handler->GPIO_Port, (uint32_t) Data);

	return (SUCCESS);
}

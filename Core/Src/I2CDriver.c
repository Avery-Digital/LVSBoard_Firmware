/*******************************************************************************
 * @file    Src/I2CDriver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   I2C Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "I2CDriver.h"

/* Public functions ----------------------------------------------------------*/
ErrorStatus I2CInit (I2CHandler *I2C_Handler )
{
	ErrorStatus Status = SUCCESS;

	/* Enable clock for I2C's GPIO0 */
	LL_AHB4_GRP1_EnableClock( I2C_Handler->I2C_GPIO0_Clk);

	/* Configure I2C's GPIO0 */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin				= I2C_Handler->I2C_GPIO0_Pin;
    GPIO_InitStruct.Mode			= LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed			= LL_GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.OutputType		= LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull			= LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   	= I2C_Handler->I2C_GPIO0_AF;
    LL_GPIO_Init ( I2C_Handler->I2C_GPIO0_Port, &GPIO_InitStruct);

    if ( I2C_Handler->I2C_CMPLX_Port == TRUE)
    {
    	/* Enable clock for I2C's GPIO1 */
    	LL_AHB4_GRP1_EnableClock( I2C_Handler->I2C_GPIO1_Clk);

    	/* Configure I2C's GPIO1 */
    	GPIO_InitStruct.Pin			= I2C_Handler->I2C_GPIO1_Pin;
        GPIO_InitStruct.Alternate	= I2C_Handler->I2C_GPIO1_AF;
    	LL_GPIO_Init ( I2C_Handler->I2C_GPIO1_Port, &GPIO_InitStruct);
    }

	/* Enable clock for I2C */
    LL_RCC_SetSPIClockSource( I2C_Handler->I2C_Clk_Source);
    /* Enable peripheral bus */
    switch( I2C_Handler->I2C_Bus)
    {
    case APB1:
    	LL_APB1_GRP1_EnableClock( I2C_Handler->I2C_Clk);
    	break;
    case APB2:
    	LL_APB2_GRP1_EnableClock( I2C_Handler->I2C_Clk);
    	break;
    case APB3:
    	LL_APB3_GRP1_EnableClock( I2C_Handler->I2C_Clk);
    	break;
    case APB4:
    	LL_APB4_GRP1_EnableClock( I2C_Handler->I2C_Clk);
    	break;
    default:
    	Status += ERROR;
    	break;
    }

    /* Configure I2C peripheral */
	LL_I2C_InitTypeDef I2C_InitStruct;
	I2C_InitStruct.PeripheralMode		= LL_I2C_MODE_I2C;
	I2C_InitStruct.Timing				= I2C_Handler->I2C_Timing;
	I2C_InitStruct.AnalogFilter			= I2C_Handler->I2C_AnalogFilter;
	I2C_InitStruct.DigitalFilter		= I2C_Handler->I2C_DigitalFilter;
	I2C_InitStruct.OwnAddress1			= 0x00;
	I2C_InitStruct.TypeAcknowledge		= I2C_Handler->I2C_Achnowledgement;
	I2C_InitStruct.OwnAddrSize			= I2C_Handler->I2C_Addressing_Mode;
	LL_I2C_Init (I2C_Handler->I2C_Peripheral, &I2C_InitStruct);

	/* Enable I2C */
	LL_I2C_Enable(I2C_Handler->I2C_Peripheral);

    return Status;
}

ErrorStatus I2CWriteData (	I2CHandler* I2C_Handler, uint8_t I2C_Address, uint8_t* Data, uint8_t ByteNum)
{
	uint8_t i = 0;

	/* Configure the I2C in master write mode and generate a start condition */
	LL_I2C_HandleTransfer( I2C_Handler->I2C_Peripheral, I2C_Address, I2C_Handler->I2C_Addressing_Mode,
						ByteNum, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

	while(i < ByteNum)
	{
		/* Wait for TXDR ready flag */
		while(!LL_I2C_IsActiveFlag_TXIS( I2C_Handler->I2C_Peripheral));

		/* Transmit Data */
		LL_I2C_TransmitData8( I2C_Handler->I2C_Peripheral, Data[i]);
		i++;
	}

	/* Wait for end of the transmission */
	while(!LL_I2C_IsActiveFlag_STOP( I2C_Handler->I2C_Peripheral));
	LL_I2C_ClearFlag_STOP( I2C_Handler->I2C_Peripheral);

    return (SUCCESS);
}

ErrorStatus I2CReadData ( I2CHandler* I2C_Handler, uint8_t I2C_Address, uint8_t* Data, uint8_t ByteNum)
{
	uint8_t i = 0;

	/* Configure the I2C in master read mode and generate a start condition */
	LL_I2C_HandleTransfer( I2C_Handler->I2C_Peripheral, I2C_Address, LL_I2C_ADDRSLAVE_7BIT,
						ByteNum, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

	while(i < ByteNum)
	{
		/* Wait for RXNE ready flag */
		while(!LL_I2C_IsActiveFlag_RXNE( I2C_Handler->I2C_Peripheral));

		/* Receive Data */
		Data[i] = LL_I2C_ReceiveData8( I2C_Handler->I2C_Peripheral);
		i++;
	}

	/* Wait for end of the reception */
	while(!LL_I2C_IsActiveFlag_STOP(I2C_Handler->I2C_Peripheral));
	LL_I2C_ClearFlag_STOP( I2C_Handler->I2C_Peripheral);

    return (SUCCESS);
}

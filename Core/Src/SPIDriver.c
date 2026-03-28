/*******************************************************************************
 * @file    Src/SPIDriver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   SPI Driver functions
 *******************************************************************************
 * Copyright (c) 2023 Avery Digital Data
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "SPIDriver.h"
#include "stm32h7xx_ll_utils.h"

/* Public functions ----------------------------------------------------------*/
ErrorStatus SPIInit( SPIHandler *SPI_Handler)
{
	ErrorStatus Status = SUCCESS;

	/* Enable clock for SPI's GPIO0 */
	LL_AHB4_GRP1_EnableClock( SPI_Handler->SPI_GPIO0_Clk);

	/* Configure SPI's GPIO0 */
    LL_GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin			= SPI_Handler->SPI_GPIO0_Pin;
    GPIO_InitStruct.Mode		= LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed		= LL_GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.OutputType	= LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull		= LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate   = SPI_Handler->SPI_GPIO0_AF;
    LL_GPIO_Init ( SPI_Handler->SPI_GPIO0_Port, &GPIO_InitStruct);

    if ( SPI_Handler->SPI_CMPLX_Port == TRUE)
    {
    	/* Enable clock for SPI's GPIO1 */
    	LL_AHB4_GRP1_EnableClock( SPI_Handler->SPI_GPIO1_Clk);

    	/* Configure SPI's GPIO1 */
    	GPIO_InitStruct.Pin			= SPI_Handler->SPI_GPIO1_Pin;
    	GPIO_InitStruct.Alternate   = SPI_Handler->SPI_GPIO1_AF;
    	LL_GPIO_Init ( SPI_Handler->SPI_GPIO1_Port, &GPIO_InitStruct);

    	/* Enable clock for SPI's GPIO2 */
    	LL_AHB4_GRP1_EnableClock( SPI_Handler->SPI_GPIO2_Clk);

    	/* Configure SPI's GPIO2 */
    	GPIO_InitStruct.Pin			= SPI_Handler->SPI_GPIO2_Pin;
    	GPIO_InitStruct.Alternate   = SPI_Handler->SPI_GPIO2_AF;
    	LL_GPIO_Init ( SPI_Handler->SPI_GPIO2_Port, &GPIO_InitStruct);
    }

	/* Enable clock for SPI */
	LL_RCC_SetSPIClockSource( SPI_Handler->SPI_Clk_Source);

    /* Enable peripheral bus */
    switch( SPI_Handler->SPI_Bus)
    {
    case APB1:
    	LL_APB1_GRP1_EnableClock( SPI_Handler->SPI_Clk);
    	break;
    case APB2:
    	LL_APB2_GRP1_EnableClock( SPI_Handler->SPI_Clk);
    	break;
    case APB3:
    	LL_APB3_GRP1_EnableClock( SPI_Handler->SPI_Clk);
    	break;
    case APB4:
    	LL_APB4_GRP1_EnableClock( SPI_Handler->SPI_Clk);
    	break;
    default:
    	return (ERROR);
    	break;
    }

    /* Configure SPI peripheral */
    LL_SPI_InitTypeDef SPI_InitStruct;
    SPI_InitStruct.TransferDirection	= SPI_Handler->SPI_TransferDirection;
    SPI_InitStruct.Mode					= LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth			= SPI_Handler->SPI_DataWidth;
    SPI_InitStruct.ClockPolarity		= SPI_Handler->SPI_ClockPolarity;
    SPI_InitStruct.ClockPhase			= SPI_Handler->SPI_ClockPhase;
    SPI_InitStruct.NSS					= LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate				= SPI_Handler->SPI_BaudRate;
    SPI_InitStruct.BitOrder				= SPI_Handler->SPI_BitOrder;
    SPI_InitStruct.CRCCalculation		= LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly				= 0x0;
    LL_SPI_Init( SPI_Handler->SPI_Peripheral, &SPI_InitStruct);

    /* Lock GPIO for master to avoid glitches on the clock output */
    LL_SPI_DisableGPIOControl( SPI_Handler->SPI_Peripheral);
	LL_SPI_DisableMasterRxAutoSuspend( SPI_Handler->SPI_Peripheral);
    LL_SPI_SetStandard( SPI_Handler->SPI_Peripheral, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_SetFIFOThreshold( SPI_Handler->SPI_Peripheral, LL_SPI_FIFO_TH_01DATA);
    LL_SPI_DisableNSSPulseMgt( SPI_Handler->SPI_Peripheral);

    /* Enable SPI */
	LL_SPI_Enable( SPI_Handler->SPI_Peripheral);
	LL_SPI_StartMasterTransfer( SPI_Handler->SPI_Peripheral);

    return Status;
}

ErrorStatus SPIDataTransceive( SPIHandler* SPI_Handler, uint32_t* Data_Rx, uint32_t Data_Tx)
{
	/* Waiting for the TXP signal */
	while (!LL_SPI_IsActiveFlag_TXP( SPI_Handler->SPI_Peripheral));

	/* Write the Data_Tx to the SPI */
	switch( SPI_Handler->SPI_DataWidth)
	{
	case LL_SPI_DATAWIDTH_8BIT:
		LL_SPI_TransmitData8( SPI_Handler->SPI_Peripheral,   (uint8_t) Data_Tx);
		break;
	case LL_SPI_DATAWIDTH_16BIT:
		LL_SPI_TransmitData16( SPI_Handler->SPI_Peripheral, (uint16_t) Data_Tx);
		break;
	case LL_SPI_DATAWIDTH_18BIT:
	case LL_SPI_DATAWIDTH_24BIT:
	case LL_SPI_DATAWIDTH_32BIT:
		LL_SPI_TransmitData32( SPI_Handler->SPI_Peripheral, (uint32_t) Data_Tx);
		break;
	}

	/* Waiting for the RXP signal */
	while (!LL_SPI_IsActiveFlag_RXP( SPI_Handler->SPI_Peripheral));

	/* Read the SPI Data to the Data_Rx */
	switch( SPI_Handler->SPI_DataWidth)
	{
	case LL_SPI_DATAWIDTH_8BIT:
		*Data_Rx = (uint32_t) LL_SPI_ReceiveData8( SPI_Handler->SPI_Peripheral);
		break;
	case LL_SPI_DATAWIDTH_16BIT:
		*Data_Rx = (uint32_t) LL_SPI_ReceiveData16( SPI_Handler->SPI_Peripheral);
		break;
	case LL_SPI_DATAWIDTH_18BIT:
	case LL_SPI_DATAWIDTH_24BIT:
	case LL_SPI_DATAWIDTH_32BIT:
		*Data_Rx = (uint32_t) LL_SPI_ReceiveData32( SPI_Handler->SPI_Peripheral);
		break;
	}

    return SUCCESS;
}


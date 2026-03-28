/*******************************************************************************
 * @file    Src/StatusManager.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Status Manager functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "StatusManager.h"

/* Public functions ----------------------------------------------------------*/
ErrorStatus StatusManager_Init (void)
{
	ErrorStatus Status = SUCCESS;
	Status += GPIOInit(&LED_Status_VSB0);
	Status += GPIOInit(&LED_Status_VSB1);

	return Status;
}

ErrorStatus StatusManager_Reset (void)
{
	ErrorStatus Status = SUCCESS;
	Status += GPIOReset(&LED_Status_VSB0);
	Status += GPIOReset(&LED_Status_VSB1);

	return Status;
}

ErrorStatus StatusManager_VSB0_Led( FunctionalState Condition)
{
	ErrorStatus Status = SUCCESS;
	if (Condition == ENABLE)
		Status += GPIOSet(&LED_Status_VSB0);
	else
		Status += GPIOReset(&LED_Status_VSB0);

	return Status;
}

ErrorStatus StatusManager_VSB1_Led( FunctionalState Condition)
{
	ErrorStatus Status = SUCCESS;

	if (Condition == ENABLE)
		Status += GPIOSet(&LED_Status_VSB1);
	else
		Status += GPIOReset(&LED_Status_VSB1);

	return Status;
}


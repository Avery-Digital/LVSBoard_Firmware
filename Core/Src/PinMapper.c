/*******************************************************************************
 * @file    Src/PinMapper.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Pin Mapper functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "PinMapper.h"

/* Private Variables ---------------------------------------------------------*/
static uint16_t	Temp;

/* Macros --------------------------------------------------------------------*/
#define EVENBLOCKS_ROWFACTOR		4

/* Private functions ---------------------------------------------------------*/
uint8_t PinColMAP( uint16_t SocketPin)
{
	Temp = SocketPin / ROWS_PER_COL;
	return Temp;
}

uint8_t PinRowMAP ( uint16_t SocketPin)
{
	Temp = SocketPin % ROWS_PER_COL;
	return Temp;
}

/* Todo:
 * If we need synthesis rig pin mapping we should add it here
 */

/* Public functions ----------------------------------------------------------*/
uint8_t PinBankMAP ( uint16_t SocketPin)
{
	Temp = PinRowMAP ( SocketPin);
	Temp = Temp / ROWS_PER_BANK;
	return Temp;
}

uint8_t PinBlockMAP ( uint16_t SocketPin)
{
	Temp = PinColMAP ( SocketPin);
	return Temp;
}

uint8_t PinPositionMAP (uint16_t SocketPin)
{
	uint16_t Col;
	uint16_t RowBlock, RowBlockPos;

	Col = PinColMAP ( SocketPin);
	Temp = PinRowMAP ( SocketPin);

	/* Even Columns are different */
	if (Col%2== 0)
	{
		RowBlock = Temp / EVENBLOCKS_ROWFACTOR;
		RowBlockPos = Temp % EVENBLOCKS_ROWFACTOR;
		Temp = RowBlock * EVENBLOCKS_ROWFACTOR + ((EVENBLOCKS_ROWFACTOR - 1) - RowBlockPos);
	}

	Temp = Temp % ROWS_PER_BANK;
	return Temp;
}


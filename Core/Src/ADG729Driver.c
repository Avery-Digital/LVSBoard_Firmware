/*******************************************************************************
 * @file    Src/ADG729Driver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   ADG729 Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "ADG729Driver.h"

/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
ErrorStatus ADG729Init( I2CHandler* ADG729_I2C)
{
	ErrorStatus Status = SUCCESS;

	/* Configure and enable I2C port connection for DAC */
	Status += I2CInit( ADG729_I2C);

   	return Status;
}

ErrorStatus ADG729WritePort( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Port)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Packet;
	uint8_t WrAddr, ReAddr;

	/* Prepare addresses */
	WrAddr = (Address << 1);
	ReAddr = (Address << 1) | 0x01;

	/* Write Port value to the chip */
	Packet = Port;
	Status += I2CWriteData( ADG729_I2C, WrAddr, &Packet, 1);

	/* Read Port value from the chip */
	Status += I2CReadData( ADG729_I2C, ReAddr, &Packet, 1);

	/* Verify */
	if (Packet != Port)
		Status += ERROR;

    return Status;
}

ErrorStatus ADG729ReadPort( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t* Port)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Packet;
	uint8_t ReAddr;

	/* Prepare addresses */
	ReAddr = (Address << 1) | 0x01;

	/* Read Port value from the chip */
	Status += I2CReadData( ADG729_I2C, ReAddr, &Packet, 1);

	/* Return data */
	*Port = Packet;
    return Status;
}

ErrorStatus ADG729SetPin( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Pin)
{
	ErrorStatus Status = SUCCESS;
	uint8_t	Packet = 0;
	uint8_t WrAddr, ReAddr;

	/* Prepare addresses */
	WrAddr = (Address << 1);
	ReAddr = (Address << 1) | 0x01;

	/* Acquire port state */
	Status += ADG729ReadPort( ADG729_I2C, ReAddr, &Packet);

	/* Set pin on the chip */
	Packet = Packet | (0x01 << Pin);
	Status += ADG729WritePort( ADG729_I2C, WrAddr, Packet);

	return Status;
}

ErrorStatus ADG729ResetPin( I2CHandler* ADG729_I2C, uint8_t Address, uint8_t Pin)
{
	ErrorStatus Status = SUCCESS;
	uint8_t	Packet = 0;
	uint8_t WrAddr, ReAddr;

	/* Prepare addresses */
	WrAddr = (Address << 1);
	ReAddr = (Address << 1) | 0x01;

	/* Acquire port state */
	Status += ADG729ReadPort( ADG729_I2C, ReAddr, &Packet);

	/* Set pin on the chip */
	Packet = Packet & (~(0x01 << Pin));
	Status += ADG729WritePort( ADG729_I2C, WrAddr, Packet);

	return Status;
}

ErrorStatus ADG729SetAllPins( I2CHandler* ADG729_I2C, uint8_t Address)
{
	ErrorStatus Status = SUCCESS;
	uint8_t WrAddr;

	/* Prepare addresses */
	WrAddr = (Address << 1);

	/* Set All on the chip */
	Status += ADG729WritePort( ADG729_I2C, WrAddr, 0xFF);

	return Status;
}

ErrorStatus ADG729ResetAllPins( I2CHandler* ADG729_I2C, uint8_t Address)
{
	ErrorStatus Status = SUCCESS;
	uint8_t WrAddr;

	/* Prepare addresses */
	WrAddr = (Address << 1);

	/* Reset All on the chip */
	Status += ADG729WritePort( ADG729_I2C, WrAddr, 0x00);

	return Status;
}

ErrorStatus ADG729Check( I2CHandler* ADG729_I2C, uint8_t Address)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Packet;
	uint8_t ReAddr;

	/* Prepare addresses */
	ReAddr = (Address << 1) | 0x01;

	/* Read the port */
	Status += I2CReadData( ADG729_I2C, ReAddr, &Packet, 1);

	return Status;
}

/*******************************************************************************
 * @file    Src/EVSDriver.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   External Voltage Switcher Driver functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "EVSDriver.h"

/* Private Variables ---------------------------------------------------------*/
static uint32_t	EVSB0Data [COLS_PER_BANK] = {0};
static uint32_t	EVSB1Data [COLS_PER_BANK] = {0};
static uint32_t	Temp;
static uint32_t Packet;
static uint32_t Mask;

/* Private functions ---------------------------------------------------------*/
//static inline uint8_t flip_byte(uint8_t Data)
//{
//	Data = (Data >> 4) | (Data << 4);
//	Data = ((Data & 0xCC) >> 2) | ((Data & 0x33) << 2);
//	Data = ((Data & 0xAA) >> 1) | ((Data & 0x55) << 1);
//    return Data;
//}
//
//uint32_t flip_packet_bytewise ( uint32_t Data) {
//    return (flip_byte((Data >> 24) & 0xFF) << 24) |
//           (flip_byte((Data >> 16) & 0xFF) << 16) |
//           (flip_byte((Data >>  8) & 0xFF) <<  8) |
//           (flip_byte((Data >>  0) & 0xFF) <<  0);
//}

ErrorStatus EVSIndicatorLedControl( void)
{
	ErrorStatus Status = SUCCESS;
	uint8_t i;

	Temp = 0;
	for (i=0; i<COLS_PER_BANK; i++)
		Temp = Temp | EVSB0Data[i];

	if (Temp)
		Status += StatusManager_VSB0_Led( ENABLE);
	else
		Status += StatusManager_VSB0_Led( DISABLE);

	Temp = 0;
	for (i=0; i<COLS_PER_BANK; i++)
		Temp = Temp | EVSB1Data[i];

	if (Temp)
		Status += StatusManager_VSB1_Led( ENABLE);
	else
		Status += StatusManager_VSB1_Led( DISABLE);

	return Status;
}

ErrorStatus EVSDisAllnCS( I2CHandler *I2C_Handler)
{
	ErrorStatus Status = SUCCESS;

	/* Disconnect all nCS */
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH0Addr, 0x00);
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH1Addr, 0x00);
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH2Addr, 0x00);

	return Status;
}

ErrorStatus EVSEnAllnCS( I2CHandler *I2C_Handler)
{
	ErrorStatus Status = SUCCESS;

	/* Pass through all nCS */
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH0Addr, 0x0F);
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH1Addr, 0x0F);
	Status += ADG729WritePort( I2C_Handler, EVSADG729CH2Addr, 0x0F);

	return Status;
}

/**
 * @brief  Write a block without read-back verification.
 *         Used by EVSSetAllPins() for fast sequencing.
 */
static ErrorStatus EVSBlockWrite( I2CHandler *I2C_Handler, GPIOHandler *nCS_Handler, SPIHandler *SPI_Handler, uint8_t BlockNumber, uint32_t DataWrite)
{
	ErrorStatus Status = SUCCESS;
	uint8_t 	I2CAddress, I2CPort;
	uint32_t	Word0Write, Word1Write;
	uint32_t	Word0Read, 	Word1Read;

	switch (BlockNumber)
	{
		case 0 ... 3:	I2CAddress = EVSADG729CH0Addr;	break;
		case 4 ... 7:	I2CAddress = EVSADG729CH1Addr;	break;
		case 8 ... 9:	I2CAddress = EVSADG729CH2Addr;	break;
		default:		Status += ERROR;				break;
	}

	if (Status == SUCCESS)
	{
		Word0Write = (uint16_t) (DataWrite >> 16);
		Word1Write = (uint16_t) (DataWrite);

		Status += EVSDisAllnCS( I2C_Handler);

		I2CPort = (uint8_t)(0x11 << (BlockNumber % 4));
		Status += ADG729WritePort( I2C_Handler, I2CAddress, I2CPort);

		Status += GPIOReset (nCS_Handler);
		Status += SPIDataTransceive (SPI_Handler, &Word0Read, Word0Write);
		Status += SPIDataTransceive (SPI_Handler, &Word1Read, Word1Write);
		Status += GPIOSet (nCS_Handler);

		Status += EVSEnAllnCS ( I2C_Handler);
	}

	return Status;
}

ErrorStatus EVSBlockProgram( I2CHandler *I2C_Handler, GPIOHandler *nCS_Handler, SPIHandler *SPI_Handler,  uint8_t BlockNumber, uint32_t *DataRead, uint32_t DataWrite)
{
	ErrorStatus Status = SUCCESS;
	uint8_t 	I2CAddress, I2CPort;
	uint32_t	Word0Write, Word1Write;
	uint32_t	Word0Read, 	Word1Read;

	switch (BlockNumber)
	{
		case 0 ... 3:
			I2CAddress = EVSADG729CH0Addr;
			break;
		case 4 ... 7:
			I2CAddress = EVSADG729CH1Addr;
			break;
		case 8 ... 9:
			I2CAddress = EVSADG729CH2Addr;
			break;
		default:
			Status += ERROR;
			break;
	}

	if (Status == SUCCESS)
	{
		/* Prepare write data */
		Word0Write = (uint16_t) (DataWrite >> 16);
		Word1Write = (uint16_t) (DataWrite);
		
		/* Disable all blocks */
		Status += EVSDisAllnCS( I2C_Handler);

		/* Enable specific block */
		I2CPort = (uint8_t)(0x11 << (BlockNumber % 4));
		Status += ADG729WritePort( I2C_Handler, I2CAddress, I2CPort);

		/* Transfer the data */
		Status += GPIOReset (nCS_Handler);
		Status += SPIDataTransceive (SPI_Handler, &Word0Read, Word0Write);
		Status += SPIDataTransceive (SPI_Handler, &Word1Read, Word1Write);
		Status += GPIOSet (nCS_Handler);

		/* Connect to all nCS */
		Status += EVSEnAllnCS ( I2C_Handler);
		
		/* Prepare read data */
		*DataRead = (((uint32_t) Word0Read) << 16)
				  |	 ((uint32_t) Word1Read);
	}

	return Status;
}

ErrorStatus EVSSetSinglePinInBuffer( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t Source)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Bank, Block, Position;

	switch (Source)
	{
	case 0:
		Packet = 0x0UL;
		break;
	case 1:
		Packet = 0x1UL;
		break;
	case 2:
		Packet = 0x2UL;
		break;
	default:
		Status += ERROR;
		break;
	}

	if (Status == SUCCESS)
	{
		Bank = PinBankMAP( SocketPin);
		Block = PinBlockMAP( SocketPin);
		Position = PinPositionMAP( SocketPin);

		/* Prepare the packet */
		Mask = 0x3UL << (Position * 2);
		Packet &= 0x3UL;
		Packet = Packet << (Position * 2);

		/* Mask unused pins */
		if (Bank)
		{
			if (Block % 2)
				Packet &= 0x0FFFFFFF; /* Mask bit 30 and 31 */
			else
				Packet &= 0xF0FFFFFF; /* Mask bit 28 and 29 */
		}
		/* Program buffer */
		if (Bank)
		{
			Temp = EVSB1Data [Block];
			Temp = Temp & (~Mask);
			Packet = Temp | Packet;
			EVSB1Data [Block] = Packet;
		}
		else
		{
			Temp = EVSB0Data [Block];
			Temp = Temp & (~Mask);
			Packet = Temp | Packet;
			EVSB0Data [Block] = Packet;
		}
	}
	else
		Status += ERROR;

	return Status;
}

ErrorStatus EVSGetSinglePinFromBuffer( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t *Source)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Bank, Block, Position;
	
	Bank = PinBankMAP( SocketPin);
	Block = PinBlockMAP( SocketPin);
	Position = PinPositionMAP( SocketPin);

	if (Bank)
		Packet = EVSB1Data[Block];
	else
		Packet = EVSB0Data[Block];

	Packet = Packet >> (Position * 2);
	Packet &= 0x3UL;

	switch (Packet)
	{
	case 0x0UL:	*Source = 0;	break;
	case 0x1UL:	*Source = 1;	break;
	case 0x2UL:	*Source = 2;	break;
	default: Status += ERROR;	break;
	}

	return Status;
}
/* Public functions ----------------------------------------------------------*/
ErrorStatus EVSInit( EVSHandler *EVS_Handler)
{
	ErrorStatus Status = SUCCESS;

	/* Initializing the ADG729 controller */
	Status += ADG729Init( EVS_Handler->B0I2C_Interface);
	Status += ADG729Init( EVS_Handler->B1I2C_Interface);

	/* Initialize the GPIO Interface for ADG739 */
	Status += GPIOInit( EVS_Handler->B0nRST_Pin);
	Status += GPIOInit( EVS_Handler->B0nCS_Pin);
	Status += GPIOInit( EVS_Handler->B1nRST_Pin);
	Status += GPIOInit( EVS_Handler->B1nCS_Pin);

	/* Set the pins */
	Status += GPIOSet ( EVS_Handler->B0nRST_Pin);
	Status += GPIOSet ( EVS_Handler->B0nCS_Pin);
	Status += GPIOSet ( EVS_Handler->B1nRST_Pin);
	Status += GPIOSet ( EVS_Handler->B1nCS_Pin);

	/* Initialize the SPI Interface for ADG739 */
	Status += SPIInit( EVS_Handler->B0SPI_Interface);
	Status += SPIInit( EVS_Handler->B1SPI_Interface);

	/* Pass through all nCS */
	Status += EVSEnAllnCS( EVS_Handler->B0I2C_Interface);
	Status += EVSEnAllnCS( EVS_Handler->B1I2C_Interface);

	return Status;
}

ErrorStatus EVSReset( EVSHandler *EVS_Handler)
{
	ErrorStatus Status = SUCCESS;
	uint8_t i;

	/* Pass through all nCS */
	Status += EVSEnAllnCS( EVS_Handler->B0I2C_Interface);
	Status += EVSEnAllnCS( EVS_Handler->B1I2C_Interface);

	/* Reset all latches */
	Status += GPIOReset( EVS_Handler->B0nCS_Pin);
	Status += GPIOReset( EVS_Handler->B1nCS_Pin);
	Status += GPIOReset( EVS_Handler->B0nRST_Pin);
	Status += GPIOReset( EVS_Handler->B1nRST_Pin);
	Delay( ADG714_RSTDelay);
	Status += GPIOSet ( EVS_Handler->B0nCS_Pin);
	Status += GPIOSet ( EVS_Handler->B1nCS_Pin);
	Status += GPIOSet ( EVS_Handler->B0nRST_Pin);
	Status += GPIOSet ( EVS_Handler->B1nRST_Pin);	

	if (Status == SUCCESS)
	{
		for ( i=0; i<COLS_PER_BANK; i++)
		{
			EVSB0Data [i] = 0;
			EVSB1Data [i] = 0;
		}
	}

	/* Check for LED */
	Status += EVSIndicatorLedControl();

	return Status;
}

ErrorStatus EVSCheck( EVSHandler *EVS_Handler)
{
	ErrorStatus Status = SUCCESS;
	uint8_t i;

	/* Check the existence of the ADG729 */
	Status += ADG729Check( EVS_Handler->B0I2C_Interface, EVSADG729CH0Addr);
	Status += ADG729Check( EVS_Handler->B0I2C_Interface, EVSADG729CH1Addr);
	Status += ADG729Check( EVS_Handler->B0I2C_Interface, EVSADG729CH2Addr);
	Status += ADG729Check( EVS_Handler->B1I2C_Interface, EVSADG729CH0Addr);
	Status += ADG729Check( EVS_Handler->B1I2C_Interface, EVSADG729CH1Addr);
	Status += ADG729Check( EVS_Handler->B1I2C_Interface, EVSADG729CH2Addr);

	/* Hard testing blocks */
	for ( i=0; i<COLS_PER_BANK; i++)
		Status += EVSBlockProgram ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, i, &Packet, 0xAA55AA55UL);

	for ( i=0; i<COLS_PER_BANK; i++)
	{
		Status += EVSBlockProgram( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, i, &Packet, 0x55AA55AAUL);
		if (Packet != 0xAA55AA55)
			Status += ERROR;
		Status += EVSBlockProgram( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, i, &Packet, 0x0UL);
		if (Packet != 0x55AA55AA)
			Status += ERROR;
	}

	for ( i=0; i<COLS_PER_BANK; i++)
		Status += EVSBlockProgram ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, i, &Packet, 0xAA55AA55UL);

	for ( i=0; i<COLS_PER_BANK; i++)
	{
		Status += EVSBlockProgram( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, i, &Packet, 0x55AA55AAUL);
		if (Packet != 0xAA55AA55)
			Status += ERROR;
		Status += EVSBlockProgram( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, i, &Packet, 0x0UL);
		if (Packet != 0x55AA55AA)
			Status += ERROR;
	}

	return Status;
}

ErrorStatus EVSSetSinglePin( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t Source)
{
	ErrorStatus Status = SUCCESS;
	uint8_t Bank, Block;

	if (SocketPin<TOTAL_SOCKET_PINS)
	{
		/* Prepare data */
		Status += EVSSetSinglePinInBuffer( EVS_Handler, SocketPin, Source);

		/* Find the bank and block number */
		Bank = PinBankMAP( SocketPin);
		Block = PinBlockMAP( SocketPin);
	}
	else
		Status += ERROR;

	/* Program  Switch */
	if (Status == SUCCESS)
	{
		if (Bank)
		{
			Status += EVSBlockProgram ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, Block, &Temp, EVSB1Data [Block]);
			Status += EVSBlockProgram ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, Block, &Temp, EVSB1Data [Block]);
			if (EVSB1Data [Block] != Temp)
				Status += ERROR;
		}
		else
		{
			Status += EVSBlockProgram ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, Block, &Temp, EVSB0Data [Block]);
			Status += EVSBlockProgram ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, Block, &Temp, EVSB0Data [Block]);
			if (EVSB0Data [Block] != Temp)
				Status += ERROR;
		}
	}
	else
		Status += ERROR;

	/* Check for LED */
	Status += EVSIndicatorLedControl();

	return Status;
}

ErrorStatus EVSGetSinglePin( EVSHandler *EVS_Handler, uint16_t SocketPin, uint8_t *Source)
{
	ErrorStatus Status = SUCCESS;

	if (SocketPin<TOTAL_SOCKET_PINS)
		Status += EVSGetSinglePinFromBuffer( EVS_Handler, SocketPin, Source);
	else
		Status += ERROR;

	return Status;
}

/*
 * Todo:
 * To boost the speed, make a local full socket programming functions.
 */

ErrorStatus EVSSetAllPins( EVSHandler *EVS_Handler, uint8_t *SourceData)
{
	ErrorStatus Status = SUCCESS;
	uint16_t SocketPin;
	uint8_t Block;

	/* Snapshot previous block data before updating buffers */
	uint32_t PrevB0[COLS_PER_BANK];
	uint32_t PrevB1[COLS_PER_BANK];
	for ( Block=0; Block<COLS_PER_BANK; Block++)
	{
		PrevB0[Block] = EVSB0Data[Block];
		PrevB1[Block] = EVSB1Data[Block];
	}

	/* Update software buffers with new source data */
	for ( SocketPin=0; SocketPin<TOTAL_SOCKET_PINS; SocketPin++)
		Status += EVSSetSinglePinInBuffer ( EVS_Handler, SocketPin, SourceData[SocketPin]);

	if (Status == SUCCESS)
	{
		for ( Block=0; Block<COLS_PER_BANK; Block++)
		{
			/* Only program Bank 0 block if data changed */
			if (EVSB0Data[Block] != PrevB0[Block])
				Status += EVSBlockWrite ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, Block, EVSB0Data [Block]);

			/* Only program Bank 1 block if data changed */
			if (EVSB1Data[Block] != PrevB1[Block])
				Status += EVSBlockWrite ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, Block, EVSB1Data [Block]);
		}
	}

	/* Check for LED */
	Status += EVSIndicatorLedControl();

	return Status;
}

ErrorStatus EVSGetAllPins( EVSHandler *EVS_Handler, uint8_t *SourceData)
{
	ErrorStatus Status = SUCCESS;
	uint16_t SocketPin;

	for ( SocketPin=0; SocketPin<TOTAL_SOCKET_PINS; SocketPin++)
		Status += EVSGetSinglePinFromBuffer( EVS_Handler, SocketPin, &SourceData[SocketPin]);

	return Status;
}

ErrorStatus EVSSetAllPinsSameSource( EVSHandler *EVS_Handler, uint8_t Source)
{
	ErrorStatus Status = SUCCESS;
	uint16_t SocketPin, Block;

	for ( SocketPin=0; SocketPin<TOTAL_SOCKET_PINS; SocketPin++)
		Status += EVSSetSinglePinInBuffer ( EVS_Handler, SocketPin, Source);

	if (Status == SUCCESS)
	{
		for ( Block=0; Block<COLS_PER_BANK; Block++)
		{
			Status += EVSBlockProgram ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, Block, &Temp, EVSB0Data [Block]);
			Status += EVSBlockProgram ( EVS_Handler->B0I2C_Interface, EVS_Handler->B0nCS_Pin, EVS_Handler->B0SPI_Interface, Block, &Temp, EVSB0Data [Block]);
			if (EVSB0Data [Block] != Temp)
				Status += ERROR;
			Status += EVSBlockProgram ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, Block, &Temp, EVSB1Data [Block]);
			Status += EVSBlockProgram ( EVS_Handler->B1I2C_Interface, EVS_Handler->B1nCS_Pin, EVS_Handler->B1SPI_Interface, Block, &Temp, EVSB1Data [Block]);
			if (EVSB1Data [Block] != Temp)
				Status += ERROR;
		}
	}

	/* Check for LED */
	Status += EVSIndicatorLedControl();

	return Status;
}

ErrorStatus EVSReportSourceConnection( EVSHandler *EVS_Handler, uint8_t Source, uint32_t *Number)
{
	ErrorStatus Status = SUCCESS;
	uint16_t SocketPin;
	uint8_t PinSource;

	*Number = 0;

	for (SocketPin=0; SocketPin<TOTAL_SOCKET_PINS; SocketPin++)
	{
		/* Acquire Pin Source */
		EVSGetSinglePin( EVS_Handler, SocketPin, &PinSource);
		if (Source == PinSource) *Number += 1;
	}

	return Status;
}

/*******************************************************************************
 * @file    Src/MODBUSInterface.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   MODBUS Interface functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "MODBUSInterface.h"

/* Private Variables ---------------------------------------------------------*/
static uint8_t	SourceData[TOTAL_SOCKET_PINS];
static uint8_t	MBAddr;
static uint8_t	MBFun[2];
static uint8_t	MBData[FT231_Buffer];
static uint32_t MBDataLength;
static uint8_t	MBLRC;
static char		MBBuffer[FT231_Buffer];

/* Private functions ---------------------------------------------------------*/
/* Note:
 * The character capitalization has been done in FT231Driver
 */
static inline uint8_t MBBYTE2HEX( const char *MBBytes)
{
	uint8_t HighN, LowN;

	HighN = (MBBytes[0] <= '9') ? (MBBytes[0] - '0') : (10 + (MBBytes[0] - 'A'));
    LowN  = (MBBytes[1] <= '9') ? (MBBytes[1] - '0') : (10 + (MBBytes[1] - 'A'));
    return ((HighN << 4) | LowN);
}

static inline void HEX2MBBYTE( uint8_t Hex, char *MBBytes)
{
	uint8_t HighN, LowN;

	HighN = (Hex >> 4) & 0x0F;
	LowN  = Hex & 0x0F;

	MBBytes[0] = (HighN < 10) ? (HighN + '0') : (HighN - 10 + 'A');
	MBBytes[1] = ( LowN < 10) ? (LowN  + '0') : ( LowN - 10 + 'A');
}

static inline uint16_t UINT8to16( uint8_t HighB, uint8_t LowB)
{
	uint16_t Data;

	Data = ((uint16_t) HighB) << 8;
	Data |= ((uint16_t) LowB) & 0x00FF;

	return Data;
}

static inline void UINT16to8( uint16_t Number, uint8_t *HighB, uint8_t *LowB)
{
	*HighB = (uint8_t) (Number >> 8);
	 *LowB = (uint8_t) (Number & 0x00FF);
}


static inline ErrorStatus MBBYTEisHEX( char C)
{
	ErrorStatus Status;

	switch (C)
	{
	case '0' ... '9':
	case 'A' ... 'F':
		Status = SUCCESS;
		break;
	default:
		Status = ERROR;
		break;
	}

    return Status;
}

ErrorStatus MODBUSInterfacePacketLRCCalc( uint8_t *LRC)
{
    int		Sum = 0;
	uint32_t	i=0;

    /* Add address */
    Sum += MBAddr;

    /* Add function */
    Sum += MBFun[0];
    Sum += MBFun[1];

    /* Add data */
	for ( i=0; i<MBDataLength; i++)
			Sum += MBData[i];

    *LRC = (uint8_t)(-Sum);

    return SUCCESS;
}

ErrorStatus MODBUSAnswerGen( void)
{
	ErrorStatus Status = SUCCESS;
	uint32_t i;

	/* MODBUS Indicator */
	MBBuffer[0] = ':';

	/* Address generator  */
	HEX2MBBYTE ( MBAddr, &MBBuffer[1]);

	/* Function generator */
	HEX2MBBYTE ( MBFun[0], &MBBuffer[3]);
	HEX2MBBYTE ( MBFun[1], &MBBuffer[5]);

	/* Data generator */
	i = 0;
	if (MBDataLength > 0)
		while (i<MBDataLength)
		{
			HEX2MBBYTE ( MBData[i], &MBBuffer[7+i*2]);
			i++;
		}

	/* LRC generator */
	HEX2MBBYTE ( MBLRC, &MBBuffer[7+i*2]);

	/* Null the end */
	MBBuffer[9+i*2] = 0x0;

	return Status;
}

/* Public functions ----------------------------------------------------------*/
ErrorStatus MODBUSInterfacePacketAddrCheck( void)
{
	ErrorStatus Status = SUCCESS;

	Status += MBBYTEisHEX (Message[1]);
	Status += MBBYTEisHEX (Message[2]);

	if (MBBYTE2HEX( &Message[1]) != BoardAddrMB)
		Status += ERROR;

	return Status;
}


ErrorStatus MODBUSInterfacePacketLRCCheck( void)
{
	ErrorStatus Status = SUCCESS;
	uint8_t CalcedLRC;

	Status += MODBUSInterfacePacketLRCCalc( &CalcedLRC);
	if (MBLRC != CalcedLRC)
		Status += ERROR;

	return Status;
}

ErrorStatus MODBUSInterfacePacketSizeCheck( void)
{
	ErrorStatus Status = SUCCESS;

	if (MessageSize<9)
		Status += ERROR;

	if ((MessageSize% 2) == 0)
		Status += ERROR;

	return Status;
}

ErrorStatus MODBUSInterfacePacketHexCheck( void)
{
	uint32_t i;

    // Start after the ':'
    for ( i=1; i<MessageSize; ++i)
    	if (MBBYTEisHEX (Message[i]) != SUCCESS)
    	    return ERROR;

    return SUCCESS;
}

ErrorStatus MODBUSInterfacePacketParser( void)
{
	ErrorStatus Status = SUCCESS;
	uint32_t	i=0;

	/* Parse the address */
	MBAddr 	= MBBYTE2HEX( &Message[1]);

	/* Parse the function */
	MBFun[0]= MBBYTE2HEX( &Message[3]);
	MBFun[1]= MBBYTE2HEX( &Message[5]);

	/* Parse the data */
	MBDataLength = (MessageSize - 9)/2;
	i = 0;
	if (MBDataLength > 0)
		while (i<MBDataLength)
		{
			MBData[i] = MBBYTE2HEX( &Message[7+i*2]);
			i++;
		}
	MBData [i]=0x0;

	/* Parse the LRC */
	MBLRC	= MBBYTE2HEX( &Message[MessageSize-2]);

	return Status;
}

ErrorStatus MODBUSInterfaceReport( void)
{
	ErrorStatus Status = SUCCESS;

	/* Generate the LRC*/
	MODBUSInterfacePacketLRCCalc (&MBLRC);

	/* Generate the message */
	MODBUSAnswerGen();
	FT231Answer( MBBuffer);

	return Status;
}

ErrorStatus MODBUSInterfaceErrorReport( uint16_t MBERROR)
{
	ErrorStatus Status = SUCCESS;

	/* Generate the data*/
	MBDataLength=2;
	MBData[0]=MBFun[0];
	MBData[1]=MBFun[1];

	/* Generate the function */
	MBFun[0] = (uint8_t) (MBERROR >> 8);
	MBFun[1] = (uint8_t) (MBERROR & 0x00FF);

	/* Generate the LRC*/
	MODBUSInterfacePacketLRCCalc (&MBLRC);

	/* Generate the message */
	MODBUSAnswerGen();
	FT231Answer( MBBuffer);

	return Status;
}

ErrorStatus MBVoltageSwitcher( uint16_t CommandFunc)
{
	ErrorStatus Status = SUCCESS;
	uint16_t	SwitchNumber;
	uint8_t		SwitchConnect;

	switch(CommandFunc)
	{
	/* Bulk configuration */
	case AllSwitchesFloatMBF:
		if (MBDataLength == 0)
		{
			Status += EVSSetAllPinsSameSource( &VS_ADG714, 0);
			if (Status == SUCCESS)
			{
				strcpy((char*)MBData, "DONE");
				MBDataLength = (uint32_t) strlen("DONE");
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	case AllSwitchesToVIn1MBF:
		if (MBDataLength == 0)
		{
			Status += EVSSetAllPinsSameSource( &VS_ADG714, 1);
			if (Status == SUCCESS)
			{
				strcpy((char*)MBData, "DONE");
				MBDataLength = (uint32_t) strlen("DONE");
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	case AllSwitchesToVIn2MBF:
		if (MBDataLength == 0)
		{
			Status += EVSSetAllPinsSameSource( &VS_ADG714, 2);
			if (Status == SUCCESS)
			{
				strcpy((char*)MBData, "DONE");
				MBDataLength = (uint32_t) strlen("DONE");
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	/* Individual switch configuration */
	case SetSingleSwitchMBF:
		if (MBDataLength == 3)
		{
			SwitchNumber	= UINT8to16( MBData[0], MBData[1]);
			SwitchConnect	= MBData[2];
			Status += EVSSetSinglePin( &VS_ADG714, SwitchNumber, SwitchConnect);
			if (Status == SUCCESS)
			{
				strcpy((char*)MBData, "DONE");
				MBDataLength = (uint32_t) strlen("DONE");
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	case GetSingleSwitchMBF:
		if (MBDataLength == 2)
		{
			SwitchNumber	= UINT8to16( MBData[0], MBData[1]);
			Status += EVSGetSinglePin( &VS_ADG714, SwitchNumber, &SwitchConnect);
			if (Status == SUCCESS)
			{
				MBDataLength = 3;
				MBData[2]	 = SwitchConnect;
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	case SetAllSwitchesMBF:
		if (MBDataLength == 300)
		{
			for ( SwitchNumber=0; SwitchNumber<TOTAL_SOCKET_PINS; SwitchNumber++)
				SourceData[SwitchNumber] = MBData[SwitchNumber];
			Status += EVSSetAllPins( &VS_ADG714, SourceData);
			if (Status == SUCCESS)
			{
				strcpy((char*)MBData, "DONE");
				MBDataLength = (uint32_t) strlen("DONE");
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	case GetAllSwitchesMBF:
		if (MBDataLength == 0)
		{
			Status += EVSGetAllPins( &VS_ADG714, SourceData);
			if (Status == SUCCESS)
			{
				MBDataLength = 300;
				for ( SwitchNumber=0; SwitchNumber<TOTAL_SOCKET_PINS; SwitchNumber++)
					MBData[SwitchNumber] = SourceData[SwitchNumber];
				MODBUSInterfaceReport();
			}
			else
				MODBUSInterfaceErrorReport( FunExeErrorMBF);
		}
		else
			MODBUSInterfaceErrorReport( FunArgSizeErrorMBF);
		break;
	default:
		Status += MODBUSInterfaceErrorReport( FunErrorMBF);
		break;
	}

	return SUCCESS;
}

ErrorStatus MODBUSInterCommandDispatcher( void)
{
	ErrorStatus Status = SUCCESS;
	uint16_t	CommandFunc;

	CommandFunc = ((uint16_t)MBFun[0] << 8) | ((uint16_t)MBFun[1] & 0x00FF);

	switch (CommandFunc)
	{
	case AllSwitchesFloatMBF	:
	case AllSwitchesToVIn1MBF	:
	case AllSwitchesToVIn2MBF	:
	case SetSingleSwitchMBF		:
	case GetSingleSwitchMBF		:
	case SetAllSwitchesMBF		:
	case GetAllSwitchesMBF		:
		Status += MBVoltageSwitcher( CommandFunc);
		break;
	case IDRMBF					:
		strcpy((char*)MBData, BoardIDMB);
		MBDataLength = (uint32_t) strlen(BoardIDMB);
		Status += MODBUSInterfaceReport();
		break;
	case FRRMBF					:
		strcpy((char*)MBData, BoardFRMB);
		MBDataLength = (uint32_t) strlen(BoardFRMB);
		Status += MODBUSInterfaceReport();
		break;
	case BISTRMBF				:
		if (BISTPass == 1)
		{
			strcpy((char*)MBData, RPASSMBF);
			MBDataLength = (uint32_t) strlen(RPASSMBF);
		}
		else
		{
			strcpy((char*)MBData, RFAILMBF);
			MBDataLength = (uint32_t) strlen(RFAILMBF);
		}
		Status += MODBUSInterfaceReport();
		break;
	default						:
		Status += MODBUSInterfaceErrorReport( FunErrorMBF);
		break;
	}
	return SUCCESS;
}

ErrorStatus MODBUSInterfaceHandler( void)
{
	ErrorStatus Status = SUCCESS;

	if (ModeBusMessage)
	{
		do
		{
			/* Note:
	     	 * Board needs at lease 3 bytes in MODBUS to be received
	     	 * and the address match to respond. Otherwise it will
	     	 * stay silent
	     	 */
			/* Check for package minimum of address */
			if (MessageSize < 3)
				Status += ERROR;
	    	Status += MODBUSInterfacePacketAddrCheck();
			if (Status != SUCCESS)
	    		break;

	    	/* Package processing */
			Status += MODBUSInterfacePacketSizeCheck();
			if (Status != SUCCESS)
			{
				MODBUSInterfaceErrorReport( SizeErrorMBF);
				break;
			}

			Status += MODBUSInterfacePacketHexCheck();
			if (Status != SUCCESS)
			{
				MODBUSInterfaceErrorReport( HexErrorMBF);
				break;
			}

			Status += MODBUSInterfacePacketParser();
			Status += MODBUSInterfacePacketLRCCheck();
			if (Status != SUCCESS)
			{
				MODBUSInterfaceErrorReport( LRCErrorMBF);
				break;
			}

			Status += MODBUSInterCommandDispatcher();
			if (Status != SUCCESS)
			{
				MODBUSInterfaceErrorReport( GeneralFailureMBF);
				break;
			}
		} while (0);
		/* Reset the USB MODBUS command flag for next MODBUS command */
		ModeBusMessage	= 0;
		MessageSize		= 0;
	}

	return Status;
}


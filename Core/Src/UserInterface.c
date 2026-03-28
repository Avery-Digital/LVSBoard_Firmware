/*******************************************************************************
 * @file    Src/UserInterface.c
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   User Interface functions
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "UserInterface.h"

/* Private Variables ---------------------------------------------------------*/
static uint16_t	Command			= IDLE;
static uint8_t 	CommandLevel	= 1;
static uint16_t	SocketPin;
static uint8_t	Source;
static uint8_t	SourceData[TOTAL_SOCKET_PINS];

#define UserBufferSize	500
static char		UserBuffer[UserBufferSize];

/* Private functions ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
ErrorStatus UserInterfaceBIST( void)
{
	ErrorStatus Status = SUCCESS;

	/* Send welcome message */
	Status += FT231AnswerWOS( "");
	Status += FT231AnswerWOS( WelcomeMessage);

	/* Checking connected devices */
	Status += FT231AnswerWOS( "Connected peripherals:");

	/* Check Voltage Switcher */
	if (EVSCheck( &VS_ADG714) == SUCCESS)
		Status += FT231AnswerWOS( "ADG714\t\t@ Voltage Switcher");
	else
	{
		Status += FT231AnswerWOS( "Error\t\t@ Voltage Switcher");
		Status += ERROR;
	}

	/* Let user know about the status */
	if (Status == SUCCESS)
		Status += FT231AnswerWOS( BISTPassMessage);
	else
		Status += FT231AnswerWOS( BISTFailMessage);

	/* Send hint message */
	Status += FT231Answer( HintMessage);

	/* Reset User Interface */
	Status += ResetUserInterface();

	return Status;
}

ErrorStatus ResetUserInterface( void)
{
	Command				= IDLE;
	CommandLevel		= 1;
	return SUCCESS;
}

ErrorStatus VoltageSwitcherInterface( void)
{
	uint32_t i, j, len;

	switch(Command)
	{
	/* Bulk configuration */
	case AllPinsFloat:
		EVSSetAllPinsSameSource( &VS_ADG714, 0);
		FT231Answer( AllPinsFloatedMessage);
		ResetUserInterface();
		break;
	case AllPinsToVIn1:
		EVSSetAllPinsSameSource( &VS_ADG714, 1);
		FT231Answer( AllPinstoVIn1Message);
		ResetUserInterface();
		break;
	case AllPinsToVIn2:
		EVSSetAllPinsSameSource( &VS_ADG714, 2);
		FT231Answer( AllPinstoVIn2Message);
		ResetUserInterface();
		break;
	/* Individual switch configuration */
	case SetSinglePin:
		switch (CommandLevel)
		{
		case 1:
			FT231Answer( EnterSinglePinMessage);
			CommandLevel ++;
			break;
		case 2:
			SocketPin = (uint16_t) atoi( Message);
			if (SocketPin < TOTAL_SOCKET_PINS)
			{
				sprintf( UserBuffer, EnterSinglePinSourceMessage, SocketPin);
				FT231Answer( UserBuffer);
				CommandLevel ++;
			}
			else
			{
				FT231Answer( "Bad pin number!");
				ResetUserInterface();
			}
			break;
		case 3:
			Source = (uint8_t) atoi( Message);
			if (Source < 3)
			{
				if (EVSSetSinglePin( &VS_ADG714, SocketPin, Source) == SUCCESS)
				{
					if (!Source)
						sprintf( UserBuffer, PinFloatedMessage, SocketPin);
					else
						sprintf( UserBuffer, PinSourceMessage, SocketPin, Source);
					FT231Answer( UserBuffer);
				}
				else
					FT231Answer( "Error @ VoltageSwitcherInterface->SetSinglePin->EVSSetSinglePin!");
			}
			else
				FT231Answer( "Bad source number!");
			ResetUserInterface();
			break;
		default:
			FT231Answer( "Bug @ VoltageSwitcherInterface->SetSinglePin!");
			ResetUserInterface ();
			break;
		}
		break;
	case GetSinglePin:
		switch (CommandLevel)
		{
		case 1:
			FT231Answer( EnterSinglePinMessage);
			CommandLevel ++;
			break;
		case 2:
			SocketPin = (uint16_t) atoi( Message);
			if (EVSGetSinglePin( &VS_ADG714, SocketPin, &Source) == SUCCESS)
			{
				if (!Source)
					sprintf( UserBuffer, PinFloatedMessage, SocketPin);
				else
					sprintf( UserBuffer, PinSourceMessage, SocketPin, Source);
				FT231Answer( UserBuffer);
			}
			else
				FT231Answer( "Error @ VoltageSwitcherInterface->GetSinglePin->EVSGetSinglePin!");
			ResetUserInterface();
			break;
		default:
			FT231Answer( "Bug @ VoltageSwitcherInterface->GetSinglePin!");
			ResetUserInterface ();
			break;
		}
		break;
	case SetAllPins:
		switch (CommandLevel)
		{
		case 1:
			FT231Answer( EnterPinsSourcesMessage);
			CommandLevel ++;
			break;
		case 2:
			if (MessageSize == TOTAL_SOCKET_PINS)
			{
				for (i=0; i<TOTAL_SOCKET_PINS; i++)
					SourceData[i] = (uint8_t) (Message[i] - '0');
				if (EVSSetAllPins( &VS_ADG714, SourceData) == SUCCESS)
					FT231Answer( AckPinsSourcesMessage);
				else
					FT231Answer( "Error @ VoltageSwitcherInterface->SetAllPins->EVSSetAllPins!");
			}
			else
				FT231Answer( "Error @ VoltageSwitcherInterface->SetAllPins->MessageSize!");
			ResetUserInterface();
			break;
		default:
			FT231Answer( "Bug @ VoltageSwitcherInterface->SetAllPins!");
			ResetUserInterface ();
			break;
		}
		break;
	case GetAllPins:
		FT231Answer( PinsSourcesMessage);
		EVSGetAllPins( &VS_ADG714, SourceData);
		for ( i=0; i<COLS_PER_BANK; i++)
		{
			len = 0;
			for ( j=0; j<ROWS_PER_COL; j++)
			    len += snprintf( UserBuffer + len, UserBufferSize-len, "%d ", SourceData[ROWS_PER_COL*i + j]);
			FT231Answer( UserBuffer);
		}
		ResetUserInterface ();
		break;
	/* Report source connection */
	case ReportSourceConn:
		switch (CommandLevel)
		{
		case 1:
			FT231Answer( EnterSourceMessage);
			CommandLevel ++;
			break;
		case 2:
			SourceData[0] = atoi(Message);
			if (EVSReportSourceConnection( &VS_ADG714, SourceData[0], &i) == SUCCESS)
				sprintf( UserBuffer, SourceConnectionMessage, SourceData[0], i);
			else
				sprintf( UserBuffer,  "Bug found @ VoltageSwitcherInterface->ReportSourceConn->EVSReportSourceConnection!");
			FT231Answer( UserBuffer);
			ResetUserInterface();
			break;
		default:
			FT231Answer( "Bug found @ VoltageSwitcherInterface->ReportSourceConn!");
			ResetUserInterface();
			break;
		}
		break;		
	default:
		FT231Answer( "Bug @ VoltageSwitcherInterface!");
		ResetUserInterface ();
		break;
	}

	return SUCCESS;
}

ErrorStatus UserInterfaceCommandDispatcher( void)
{
	if (Command == IDLE)
	{
		/* Menu Management ---------------------------------------------------*/
			 if (! strcmp( Message, HelpString))				Command = Help;
		else if (! strcmp( Message, IDRString))					Command = IDR;
		else if (! strcmp( Message, BISTProcessString))			Command = BISTProcess;
		/* Voltage Switcher Management ---------------------------------------*/
		else if (! strcmp( Message, AllPinsFloatString))		Command = AllPinsFloat;
		else if (! strcmp( Message, AllPinsToVIn1String))		Command = AllPinsToVIn1;
		else if (! strcmp( Message, AllPinsToVIn2String))		Command = AllPinsToVIn2;
		else if (! strcmp( Message, SetSinglePinString))		Command = SetSinglePin;
		else if (! strcmp( Message, GetSinglePinString))		Command = GetSinglePin;
		else if (! strcmp( Message, SetAllPinsString))			Command = SetAllPins;
		else if (! strcmp( Message, GetAllPinsString))			Command = GetAllPins;
		else if (! strcmp( Message, ReportSourceConnString))	Command = ReportSourceConn;
		/* In case of bugs ---------------------------------------------------*/
		else													Command = GeneralFailure;
	}

	switch (Command)
	{
	case AllPinsFloat		:
	case AllPinsToVIn1		:
	case AllPinsToVIn2		:
	case SetSinglePin		:
	case GetSinglePin		:
	case SetAllPins			:
	case GetAllPins			:
	case ReportSourceConn	: VoltageSwitcherInterface ();	break;
	case Help				: FT231Answer( HelpMessage);
							  ResetUserInterface();			break;
	case IDR				: FT231Answer( IDRMessage);
							  ResetUserInterface();			break;
	case BISTProcess		: UserInterfaceBIST();			break;
	default					: FT231Answer ( FailedMessage);
							  ResetUserInterface();			break;
	}
	return SUCCESS;
}

ErrorStatus UserInterfaceHandler (void)
{
	ErrorStatus Status = SUCCESS;

	if (UserMessage)
	{
		UserInterfaceCommandDispatcher ();
		/* Reset the USB user command flag for next user command */
		UserMessage	= 0;
		MessageSize	= 0;
	}

	return Status;
}




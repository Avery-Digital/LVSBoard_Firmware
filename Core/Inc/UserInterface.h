/*******************************************************************************
 * @file    Inc/UserInterface.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   User Interface header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

#ifndef __USERINTERFACE_H_
#define __USERINTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "EVSDriver.h"
#include "StatusManager.h"
#include "FT231Driver.h"

/* Macros --------------------------------------------------------------------*/
/* Menu Macros */
#define IDLE						0
#define IDLEString					"0"

/* Automation */

/* Voltage Switcher */
#define AllPinsFloat 				300
#define AllPinsFloatString			"300"
#define AllPinsToVIN0 				301
#define AllPinsToVIN0String			"301"
#define AllPinsToVIN1 				302
#define AllPinsToVIN1String			"302"

#define SetSinglePin				310
#define SetSinglePinString			"310"
#define GetSinglePin				311
#define GetSinglePinString			"311"

#define	SetAllPins					320
#define	SetAllPinsString			"320"
#define	GetAllPins					321
#define	GetAllPinsString			"321"
#define ReportSourceConn			322
#define ReportSourceConnString		"322"

#define Help						0xEFFF
#define HelpString					"?"
#define IDR							0xE001
#define IDRString					"ID"
#define BISTProcess					0xE0FF
#define BISTProcessString			"BIST"
#define GeneralFailure				0xFFFF

/* Messages Macros*/
#define	WelcomeMessage				(\
									"======================================\r\n"\
									"=       ..::  Avery 2025  ::..       =\r\n"\
									"= Low Voltage Switching Board Prompt =\r\n"\
									"--------------------------------------"\
									)

#define BISTPassMessage				(\
									"--------------------------------------\r\n"\
									"BIST procedure finished successfully."\
									)

#define BISTFailMessage				(\
									"--------------------------------------\r\n"\
									"BIST procedure failed!"\
									)

#define HintMessage					(\
									"--------------------------------------\r\n"\
									"Enter command number or ? for help!   \r\n"\
									"======================================"\
									)

#define	HelpMessage					(\
									"List of available options:\r\n"\
									"300: Float All the pins\r\n"\
									"301: Connect All the pins to VIN0\r\n"\
									"302: Connect All the pins to VIN1\r\n"\
									"310: Set single pin configuration\r\n"\
									"311: Get single pin configuration\r\n"\
									"320: Set all pins configuration\r\n"\
									"321: Get all pins configuration\r\n"\
									"322: Report source connection\r\n"\
									"ID: Prints board identification\r\n"\
									"BIST: Run built-in self test procedure\r\n"\
									" ?: Shows this message"\
						 	 	 	 )

#define IDRMessage					(\
									"Low Voltage Switching Board rev.1"\
									)

#define FailedMessage				"Unsupported command..."

#define AllPinsFloatedMessage		"All pins are floated."
#define AllPinstoVIN0Message		"All pins are connected to VIN0."
#define AllPinstoVIN1Message		"All pins are connected to VIN1."
#define EnterSinglePinMessage		"Enter pin number: (0~299)"
#define EnterSinglePinSourceMessage	"Enter pin %hu source: ( 0)Float  1)VIN0  2)VIN1 )"
#define	PinFloatedMessage			"Pin %hu is floated."
#define	PinSourceMessage			"Pin %hu is connected to VIN%d."
#define EnterPinsSourcesMessage		"Enter sources of all pins sequentially: (300 data point expected)"
#define AckPinsSourcesMessage		"Sources for all pins applied."
#define PinsSourcesMessage			"Sources of all pins sequentially: (10 blocks, 30 pins)"
#define EnterSourceMessage			"Enter target source: ( 0)Float  1)VIN0  2)VIN1 )"
#define SourceConnectionMessage		"VIN%d is connected to %lu pin(s)."


/* Public function prototypes ------------------------------------------------*/
ErrorStatus ResetUserInterface (void);
ErrorStatus UserInterfaceHandler (void);


#endif /* __USERINTERFACE_H_ */

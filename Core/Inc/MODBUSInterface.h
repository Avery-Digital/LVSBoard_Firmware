/*******************************************************************************
 * @file    Inc/MODBUSInterface.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   MODBUS Interface header
 *******************************************************************************
 * Copyright (c) 2025 Avery Bio Corporation
 * All rights reserved.
 ******************************************************************************/

#ifndef __MODBUSINTERFACE_H_
#define __MODBUSINTERFACE_H_

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "EVSDriver.h"
#include "FT231Driver.h"

/* Macros --------------------------------------------------------------------*/
/* Menu Macros */
#define IDLE						0

/* Automation */

/* Voltage Switcher */
#define AllSwitchesFloatMBF			0x0300
#define AllSwitchesToVIn1MBF		0x0301
#define AllSwitchesToVIn2MBF		0x0302

#define SetSingleSwitchMBF			0x0310
#define GetSingleSwitchMBF			0x0311

#define	SetAllSwitchesMBF			0x0320
#define	GetAllSwitchesMBF			0x0321

#define IDRMBF						0xE001
#define	FRRMBF						0xE002
#define BISTRMBF					0xE0FF

#define	SizeErrorMBF				0xF001
#define	HexErrorMBF					0xF002
#define LRCErrorMBF					0xF003
#define	FunErrorMBF					0xF004
#define FunArgSizeErrorMBF			0xF005
#define FunArgErrorMBF				0xF006

#define FunExeErrorMBF				0xFF01
#define CollisionErrorMBF			0xFFF1
#define GeneralFailureMBF			0xFFFF

#define	BoardIDMB					"LVS"
#define	BoardFRMB					"R1"
#define	RPASSMBF					"PASS"
#define	RFAILMBF					"FAIL"
#define	BoardAddrMB					0x30

/* Public function prototypes ------------------------------------------------*/
ErrorStatus MODBUSInterfaceHandler (void);

#endif /* __MODBUSINTERFACE_H_ */

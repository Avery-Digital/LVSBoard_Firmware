/*******************************************************************************
 * @file    Src/StatusManager.h
 * @author  Omid Ghadami (omid.ghadami@avery.tech)
 * @brief   Status Manager Header
 *******************************************************************************
 * Copyright (c) 2023 Avery Digital Data
 * All rights reserved.
 ******************************************************************************/

#ifndef __STATUSMANAGER_H_
#define __STATUSMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "GPIODriver.h"
#include "LVSConfig.h"

/* Macros --------------------------------------------------------------------*/
/* Public Variables ----------------------------------------------------------*/

/* Public function prototypes ------------------------------------------------*/
ErrorStatus StatusManager_Init (void);
ErrorStatus StatusManager_Reset (void);
ErrorStatus StatusManager_VSB0_Led( FunctionalState Condition);
ErrorStatus StatusManager_VSB1_Led( FunctionalState Condition);

#ifdef __cplusplus
}
#endif

#endif /* __STATUSMANAGER_H_ */

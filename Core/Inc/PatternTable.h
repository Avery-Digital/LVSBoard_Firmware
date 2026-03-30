/*******************************************************************************
 * @file    Inc/PatternTable.h
 * @brief   Pattern Table — 1000-slot lookup table and sequence runner
 *
 *          Each slot holds 300 pin states (0=Float, 1=VIN0, 2=VIN1).
 *          Slots can be uploaded at runtime via command.
 *          The sequence runner steps through (slot, delay_ms) pairs
 *          with deterministic timing managed by SysTick.
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

#ifndef PATTERN_TABLE_H
#define PATTERN_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "PinMapper.h"   /* TOTAL_SOCKET_PINS (300) */

/* Defines -------------------------------------------------------------------*/
#define PATTERN_TABLE_SLOTS     1000U
#define PATTERN_PIN_COUNT       TOTAL_SOCKET_PINS   /* 300 */
#define SEQ_MAX_STEPS           1024U
#define PATTERN_APPLY_TIME_US   12000U  /**< Fixed pattern apply time (12 ms) */

/* Types ---------------------------------------------------------------------*/
typedef struct {
    uint16_t pattern_index;
    uint16_t delay_100us;       /**< Delay in 100 µs units (0–65535 = 0–6.5535 s) */
} SeqStep;

typedef enum {
    SEQ_IDLE,
    SEQ_RUNNING,
    SEQ_STOPPING
} SeqState;

/* Public Variables ----------------------------------------------------------*/
extern uint8_t PatternTable[PATTERN_TABLE_SLOTS][PATTERN_PIN_COUNT];

/* Public API ----------------------------------------------------------------*/

/** @brief  Zero-fill all 1000 pattern slots. Called at boot. */
void PatternTable_Init(void);

/**
 * @brief  Write 300 pin states into a pattern slot.
 * @param  slot  Slot index (0–999)
 * @param  pin_states  Array of 300 bytes
 * @return true on success, false if slot out of range or sequence running
 */
bool PatternTable_WriteSlot(uint16_t slot, const uint8_t *pin_states);

/**
 * @brief  Get a pointer to a pattern slot's data.
 * @param  slot  Slot index (0–999)
 * @return Pointer to 300-byte array, or NULL if out of range
 */
const uint8_t *PatternTable_GetSlot(uint16_t slot);

/**
 * @brief  Load a sequence of (pattern, delay) steps and start execution.
 * @param  step_data  Raw payload: [pat_hi, pat_lo, del_hi, del_lo] x N
 * @param  num_steps  Number of steps (1–1024)
 * @return true if started, false on error
 */
bool Sequence_Start(const uint8_t *step_data, uint16_t num_steps);

/** @brief  Request the running sequence to stop. */
void Sequence_Stop(void);

/**
 * @brief  Cooperative state machine — call from main loop.
 *         Applies patterns and manages delays.
 */
void Sequence_Poll(void);

/** @brief  Get current sequence state. */
SeqState Sequence_GetState(void);

/** @brief  Get current step index (for status reporting). */
uint16_t Sequence_GetCurrentStep(void);

/** @brief  Get total steps in loaded sequence. */
uint16_t Sequence_GetTotalSteps(void);

#ifdef __cplusplus
}
#endif

#endif /* PATTERN_TABLE_H */

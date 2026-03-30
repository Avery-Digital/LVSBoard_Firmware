/*******************************************************************************
 * @file    Src/PatternTable.c
 * @brief   Pattern Table — storage and sequence execution
 *
 *          1000-slot pattern table (300 bytes each) in RAM.
 *          All slots start zeroed (Float).  Sequence runner uses a
 *          cooperative state machine polled from the main loop.
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "PatternTable.h"
#include "EVSDriver.h"
#include "LVSConfig.h"
#include "stm32h7xx_ll_bus.h"
#include "stm32h7xx_ll_tim.h"
#include <string.h>

/* ==========================================================================
 *  PATTERN TABLE (300 KB in D1 SRAM .bss — zeroed by CRT at startup)
 * ========================================================================== */

uint8_t PatternTable[PATTERN_TABLE_SLOTS][PATTERN_PIN_COUNT];

/* ==========================================================================
 *  HARDWARE TIMER (TIM2 — 32-bit free-running, 1 µs per tick)
 *
 *  APB1 timer clock = 256 MHz (APB1 = 128 MHz, prescaler != 1 → 2x)
 *  TIM2 prescaler = 255 → 256 MHz / 256 = 1 MHz → 1 µs per tick
 *  32-bit counter overflows after ~71 minutes (no concern)
 *
 *  Delay field in protocol: units of 100 µs (0–65535 = 0–6.5535 s)
 * ========================================================================== */

static inline uint32_t Timer_GetTick(void)
{
    return LL_TIM_GetCounter(TIM2);
}

static inline uint32_t Timer_DelayToTicks(uint16_t delay_100us)
{
    return (uint32_t)delay_100us * 100U;  /* 100 µs units → µs ticks */
}

/* ==========================================================================
 *  SEQUENCE RUNNER STATE
 * ========================================================================== */

static SeqStep          seq_steps[SEQ_MAX_STEPS];   /* 4 bytes x 1024 = 4 KB */
static uint16_t         seq_num_steps;
static uint16_t         seq_current_step;
static volatile SeqState seq_state = SEQ_IDLE;
static uint32_t         seq_step_start_tick;
static bool             seq_step_applied;

/* ==========================================================================
 *  PATTERN TABLE API
 * ========================================================================== */

void PatternTable_Init(void)
{
    memset(PatternTable, 0, sizeof(PatternTable));
    seq_state = SEQ_IDLE;

    /* Configure TIM2 as free-running 1 µs counter */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    LL_TIM_SetPrescaler(TIM2, 255);             /* 256 MHz / 256 = 1 MHz */
    LL_TIM_SetAutoReload(TIM2, 0xFFFFFFFF);     /* 32-bit max */
    LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
    LL_TIM_GenerateEvent_UPDATE(TIM2);           /* load prescaler */
    LL_TIM_EnableCounter(TIM2);
}

bool PatternTable_WriteSlot(uint16_t slot, const uint8_t *pin_states)
{
    if (slot >= PATTERN_TABLE_SLOTS)
        return false;
    if (seq_state != SEQ_IDLE)
        return false;

    memcpy(PatternTable[slot], pin_states, PATTERN_PIN_COUNT);
    return true;
}

const uint8_t *PatternTable_GetSlot(uint16_t slot)
{
    if (slot >= PATTERN_TABLE_SLOTS)
        return NULL;
    return PatternTable[slot];
}

/* ==========================================================================
 *  SEQUENCE RUNNER
 * ========================================================================== */

bool Sequence_Start(const uint8_t *step_data, uint16_t num_steps)
{
    if (num_steps == 0 || num_steps > SEQ_MAX_STEPS)
        return false;
    if (seq_state != SEQ_IDLE)
        return false;

    /* Parse raw payload into step array */
    for (uint16_t i = 0; i < num_steps; i++) {
        uint16_t offset = i * 4;
        uint16_t pat = ((uint16_t)step_data[offset] << 8) | step_data[offset + 1];
        uint16_t del = ((uint16_t)step_data[offset + 2] << 8) | step_data[offset + 3];

        if (pat >= PATTERN_TABLE_SLOTS)
            return false;

        seq_steps[i].pattern_index = pat;
        seq_steps[i].delay_100us   = del;
    }

    seq_num_steps    = num_steps;
    seq_current_step = 0;
    seq_step_applied = false;
    seq_state        = SEQ_RUNNING;   /* commit — must be last */
    return true;
}

void Sequence_Stop(void)
{
    if (seq_state == SEQ_RUNNING)
        seq_state = SEQ_STOPPING;
}

void Sequence_Poll(void)
{
    switch (seq_state) {

    case SEQ_RUNNING:
        /* Finished all steps? */
        if (seq_current_step >= seq_num_steps) {
            seq_state = SEQ_IDLE;
            return;
        }

        /* Apply pattern if not yet done for this step */
        if (!seq_step_applied) {
            seq_step_start_tick = Timer_GetTick();
            uint16_t pat = seq_steps[seq_current_step].pattern_index;
            EVSSetAllPins(&VS_ADG714, PatternTable[pat]);

            /* Pad to fixed apply time so every pattern takes exactly 12 ms */
            while ((Timer_GetTick() - seq_step_start_tick) < PATTERN_APPLY_TIME_US)
                ;

            seq_step_applied = true;
        }

        /* Wait for delay to elapse (100 µs resolution) */
        {
            uint32_t elapsed = Timer_GetTick() - seq_step_start_tick;
            uint32_t target  = Timer_DelayToTicks(seq_steps[seq_current_step].delay_100us);
            if (elapsed >= target) {
                seq_current_step++;
                seq_step_applied = false;
            }
        }
        break;

    case SEQ_STOPPING:
        seq_state = SEQ_IDLE;
        break;

    case SEQ_IDLE:
    default:
        break;
    }
}

SeqState Sequence_GetState(void)
{
    return seq_state;
}

uint16_t Sequence_GetCurrentStep(void)
{
    return seq_current_step;
}

uint16_t Sequence_GetTotalSteps(void)
{
    return seq_num_steps;
}

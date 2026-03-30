/*******************************************************************************
 * @file    Inc/CommandInterface.h
 * @brief   Command Interface — Packet-based command dispatch
 *
 *          Receives parsed packets from the protocol layer and dispatches
 *          to command handlers.
 *
 *          Uses the deferred TX pattern: ISR callback stores the response
 *          in tx_request, main loop calls CommDriver_SendPacket().
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

#ifndef COMMAND_INTERFACE_H
#define COMMAND_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "PacketProtocol.h"

/* ========================== Command Code Macros =========================== */

#define CMD_CODE(c1, c2)    ((uint16_t)((c1) << 8) | (c2))

/* ========================= Command Definitions ============================ */

/* Voltage Switcher Commands (0x0E01–0x0E07) */
#define CMD_VS_ALL_FLOAT    CMD_CODE(0x0E, 0x01)    /**< Float all switches      */
#define CMD_VS_ALL_VIN0     CMD_CODE(0x0E, 0x02)    /**< Connect all to VIN0     */
#define CMD_VS_ALL_VIN1     CMD_CODE(0x0E, 0x03)    /**< Connect all to VIN1     */
#define CMD_VS_SET_SINGLE   CMD_CODE(0x0E, 0x04)    /**< Set single switch       */
#define CMD_VS_GET_SINGLE   CMD_CODE(0x0E, 0x05)    /**< Get single switch       */
#define CMD_VS_SET_ALL      CMD_CODE(0x0E, 0x06)    /**< Set all switches        */
#define CMD_VS_GET_ALL      CMD_CODE(0x0E, 0x07)    /**< Get all switches        */

/* First and last voltage switcher command codes — used for range check */
#define CMD_VS_FIRST        CMD_VS_ALL_FLOAT
#define CMD_VS_LAST         CMD_VS_GET_ALL

/* System Commands */
#define CMD_BOARD_ID        CMD_CODE(0xE0, 0x01)    /**< Get Board ID ("LVS")    */
#define CMD_FW_REV          CMD_CODE(0xE0, 0x02)    /**< Get FW Revision ("R1")  */
#define CMD_BIST_STATUS     CMD_CODE(0xE0, 0xFF)    /**< Get BIST result         */

/* Sequence / Pattern Commands (0x0E10–0x0E13) */
#define CMD_SEQ_UPLOAD_PATTERN CMD_CODE(0x0E, 0x10)  /**< Upload a pattern slot    */
#define CMD_SEQ_RUN            CMD_CODE(0x0E, 0x11)  /**< Run a pattern sequence   */
#define CMD_SEQ_STOP           CMD_CODE(0x0E, 0x12)  /**< Stop running sequence    */
#define CMD_SEQ_STATUS         CMD_CODE(0x0E, 0x13)  /**< Query sequence state     */

/* Firmware Info */
#define CMD_GET_FW_VERSION  CMD_CODE(0x0E, 0x69)    /**< Get firmware version str */

/* Utility */
#define CMD_PING            CMD_CODE(0xDE, 0xAD)    /**< Ping / echo test        */

/* Error Codes (used as cmd1:cmd2 in error responses) */
#define CMD_ERR_EXECUTE     CMD_CODE(0xFF, 0x01)    /**< Execution error         */
#define CMD_ERR_ARG_SIZE    CMD_CODE(0xF0, 0x05)    /**< Payload size error      */
#define CMD_ERR_UNKNOWN     CMD_CODE(0xF0, 0x04)    /**< Unknown command         */

/* ========================= TX Request Structure =========================== */

/**
 * @brief  Deferred TX request — ISR populates, main loop transmits.
 */
typedef struct {
    volatile bool   pending;
    uint8_t         msg1;
    uint8_t         msg2;
    uint8_t         cmd1;
    uint8_t         cmd2;
    uint8_t         payload[PKT_MAX_PAYLOAD];
    uint16_t        length;
} CmdTxRequest;

/* ========================= Public Variables =============================== */

extern CmdTxRequest cmd_tx_request;

/* =========================== Public API =================================== */

/**
 * @brief  Initialise the command interface.
 *         Registers OnPacketReceived as the parser callback.
 */
void CommandInterface_Init(void);

/**
 * @brief  Dispatch a received packet to the appropriate command handler.
 *
 *         Called from the protocol parser callback (OnPacketReceived).
 *         Decodes the 16-bit command from header->cmd1/cmd2 and routes
 *         to the matching handler function.
 *
 * @param  header   Decoded packet header
 * @param  payload  Payload data (header->length bytes)
 */
void Command_Dispatch(const PacketHeader *header,
                      const uint8_t *payload);

#ifdef __cplusplus
}
#endif

#endif /* COMMAND_INTERFACE_H */

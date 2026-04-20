/*******************************************************************************
 * @file    Src/CommandInterface.c
 * @brief   Command Interface — Packet-based command dispatch
 *
 *          Each command handler receives the full packet header and payload.
 *          Handlers populate cmd_tx_request for deferred transmission by
 *          the main loop (never call CommDriver_SendPacket from ISR context).
 *
 *          To add a new command:
 *            1. Define the command code in CommandInterface.h
 *            2. Write a static handler function in this file
 *            3. Add a case to Command_Dispatch()
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "CommandInterface.h"
#include "CommDriver.h"
#include "EVSDriver.h"
#include "LVSConfig.h"
#include "PatternTable.h"
#include "endian_be.h"
#include <string.h>

/* ==========================================================================
 *  PUBLIC VARIABLES
 * ========================================================================== */

CmdTxRequest cmd_tx_request = { .pending = false };

/* ==========================================================================
 *  PRIVATE DATA
 * ========================================================================== */

static uint8_t source_data[TOTAL_SOCKET_PINS];

/* ==========================================================================
 *  PRIVATE HELPERS
 * ========================================================================== */

/**
 * @brief  Send a response by populating the deferred TX request.
 *         Must be called from ISR context (OnPacketReceived callback).
 *         The pending flag is written last as a commit barrier.
 */
static void SendResponse(const PacketHeader *header,
                          uint8_t cmd1, uint8_t cmd2,
                          const uint8_t *data, uint16_t len)
{
    cmd_tx_request.msg1 = header->msg1;
    cmd_tx_request.msg2 = header->msg2;
    cmd_tx_request.cmd1 = cmd1;
    cmd_tx_request.cmd2 = cmd2;
    if (data != NULL && len > 0) {
        memcpy(cmd_tx_request.payload, data, len);
    }
    cmd_tx_request.length  = len;
    cmd_tx_request.pending = true;  /* Must be last — acts as the commit */
}

/**
 * @brief  Send an error response echoing the original command bytes
 *         in the payload, with the error code as cmd1:cmd2.
 */
static void SendError(const PacketHeader *header,
                       uint8_t err_cmd1, uint8_t err_cmd2)
{
    uint8_t err_payload[2] = { header->cmd1, header->cmd2 };
    SendResponse(header, err_cmd1, err_cmd2, err_payload, 2);
}

/* ==========================================================================
 *  VOLTAGE SWITCHER COMMAND HANDLERS
 * ========================================================================== */

static void Command_HandleVS_AllSameSource(const PacketHeader *header,
                                            const uint8_t *payload,
                                            uint8_t source)
{
    (void)payload;

    if (header->length != 0) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    ErrorStatus status = EVSSetAllPinsSameSource(&VS_ADG714, source);
    uint8_t resp = (status == SUCCESS) ? 0x00 : 0xFF;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleVS_SetSingle(const PacketHeader *header,
                                         const uint8_t *payload)
{
    if (header->length != 3) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    uint16_t pin = be16_unpack(&payload[0]);
    uint8_t  source = payload[2];

    ErrorStatus status = EVSSetSinglePin(&VS_ADG714, pin, source);
    uint8_t resp = (status == SUCCESS) ? 0x00 : 0xFF;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleVS_GetSingle(const PacketHeader *header,
                                         const uint8_t *payload)
{
    if (header->length != 2) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    uint16_t pin = be16_unpack(&payload[0]);
    uint8_t  source = 0;

    ErrorStatus status = EVSGetSinglePin(&VS_ADG714, pin, &source);

    if (status == SUCCESS) {
        uint8_t resp[3] = { payload[0], payload[1], source };
        SendResponse(header, header->cmd1, header->cmd2, resp, 3);
    } else {
        SendError(header, 0xFF, 0x01);
    }
}

static void Command_HandleVS_SetAll(const PacketHeader *header,
                                      const uint8_t *payload)
{
    if (header->length != TOTAL_SOCKET_PINS) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    memcpy(source_data, payload, TOTAL_SOCKET_PINS);
    ErrorStatus status = EVSSetAllPins(&VS_ADG714, source_data);
    uint8_t resp = (status == SUCCESS) ? 0x00 : 0xFF;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleVS_GetAll(const PacketHeader *header,
                                      const uint8_t *payload)
{
    (void)payload;

    if (header->length != 0) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    ErrorStatus status = EVSGetAllPins(&VS_ADG714, source_data);

    if (status == SUCCESS) {
        SendResponse(header, header->cmd1, header->cmd2,
                     source_data, TOTAL_SOCKET_PINS);
    } else {
        SendError(header, 0xFF, 0x01);
    }
}

/* ==========================================================================
 *  SYSTEM COMMAND HANDLERS
 * ========================================================================== */

static void Command_HandleGetFWVersion(const PacketHeader *header)
{
    static const uint8_t ver[] = "LVSBoard v2.0.0";
    SendResponse(header, header->cmd1, header->cmd2, ver, sizeof(ver) - 1);
}

static void Command_HandleBoardID(const PacketHeader *header)
{
    static const uint8_t id[] = { 'L', 'V', 'S' };
    SendResponse(header, header->cmd1, header->cmd2, id, 3);
}

static void Command_HandleFWRevision(const PacketHeader *header)
{
    static const uint8_t rev[] = { 'R', '1' };
    SendResponse(header, header->cmd1, header->cmd2, rev, 2);
}

static void Command_HandleBISTStatus(const PacketHeader *header)
{
    if (BISTPass) {
        static const uint8_t pass[] = { 'P', 'A', 'S', 'S' };
        SendResponse(header, header->cmd1, header->cmd2, pass, 4);
    } else {
        static const uint8_t fail[] = { 'F', 'A', 'I', 'L' };
        SendResponse(header, header->cmd1, header->cmd2, fail, 4);
    }
}

/* ==========================================================================
 *  SEQUENCE / PATTERN COMMAND HANDLERS
 * ========================================================================== */

static void Command_HandleSeqUploadPattern(const PacketHeader *header,
                                            const uint8_t *payload)
{
    /* Expect 2 bytes slot + 300 bytes pin states = 302 */
    if (header->length != (2 + PATTERN_PIN_COUNT)) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    uint16_t slot = be16_unpack(&payload[0]);

    if (!PatternTable_WriteSlot(slot, &payload[2])) {
        SendError(header, 0xFF, 0x01);
        return;
    }

    uint8_t resp = 0x00;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleSeqRun(const PacketHeader *header,
                                  const uint8_t *payload)
{
    /* Payload must be a multiple of 4 bytes and non-empty */
    if (header->length == 0 || (header->length % 4) != 0) {
        SendError(header, 0xF0, 0x05);
        return;
    }

    uint16_t num_steps = header->length / 4;

    if (!Sequence_Start(payload, num_steps)) {
        SendError(header, 0xFF, 0x01);
        return;
    }

    uint8_t resp = 0x00;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleSeqStop(const PacketHeader *header)
{
    Sequence_Stop();
    uint8_t resp = 0x00;
    SendResponse(header, header->cmd1, header->cmd2, &resp, 1);
}

static void Command_HandleSeqStatus(const PacketHeader *header)
{
    uint8_t  state = (uint8_t)Sequence_GetState();
    uint16_t step  = Sequence_GetCurrentStep();
    uint16_t total = Sequence_GetTotalSteps();

    uint8_t resp[5];
    resp[0] = state;
    be16_pack(&resp[1], step);
    be16_pack(&resp[3], total);
    SendResponse(header, header->cmd1, header->cmd2, resp, 5);
}

/* ==========================================================================
 *  UTILITY COMMAND HANDLERS
 * ========================================================================== */

static void Command_HandlePing(const PacketHeader *header,
                                const uint8_t *payload)
{
    /* Echo back the received payload */
    SendResponse(header, header->cmd1, header->cmd2,
                 payload, header->length);
}

/* ==========================================================================
 *  COMMAND DISPATCH
 *
 *  Combines cmd1 (high byte) and cmd2 (low byte) into a 16-bit command
 *  code and routes to the matching handler.  Unknown commands get a
 *  CMD_ERR_UNKNOWN error response.
 * ========================================================================== */

void Command_Dispatch(const PacketHeader *header,
                      const uint8_t *payload)
{
    uint16_t cmd = CMD_CODE(header->cmd1, header->cmd2);

    switch (cmd) {

    /* ---- Voltage Switcher Commands (0x0E01–0x0E07) ---- */
    case CMD_VS_ALL_FLOAT:
        Command_HandleVS_AllSameSource(header, payload, 0);
        break;
    case CMD_VS_ALL_VIN0:
        Command_HandleVS_AllSameSource(header, payload, 1);
        break;
    case CMD_VS_ALL_VIN1:
        Command_HandleVS_AllSameSource(header, payload, 2);
        break;
    case CMD_VS_SET_SINGLE:
        Command_HandleVS_SetSingle(header, payload);
        break;
    case CMD_VS_GET_SINGLE:
        Command_HandleVS_GetSingle(header, payload);
        break;
    case CMD_VS_SET_ALL:
        Command_HandleVS_SetAll(header, payload);
        break;
    case CMD_VS_GET_ALL:
        Command_HandleVS_GetAll(header, payload);
        break;

    /* ---- Sequence / Pattern Commands (0x0E10–0x0E13) ---- */
    case CMD_SEQ_UPLOAD_PATTERN:
        Command_HandleSeqUploadPattern(header, payload);
        break;
    case CMD_SEQ_RUN:
        Command_HandleSeqRun(header, payload);
        break;
    case CMD_SEQ_STOP:
        Command_HandleSeqStop(header);
        break;
    case CMD_SEQ_STATUS:
        Command_HandleSeqStatus(header);
        break;

    /* ---- Firmware Info ---- */
    case CMD_GET_FW_VERSION:
        Command_HandleGetFWVersion(header);
        break;

    /* ---- System Commands ---- */
    case CMD_BOARD_ID:
        Command_HandleBoardID(header);
        break;
    case CMD_FW_REV:
        Command_HandleFWRevision(header);
        break;
    case CMD_BIST_STATUS:
        Command_HandleBISTStatus(header);
        break;

    /* ---- Utility ---- */
    case CMD_PING:
        Command_HandlePing(header, payload);
        break;

    /* ---- Unknown Command ---- */
    default:
        SendError(header, 0xF0, 0x04);
        break;
    }
}

/* ==========================================================================
 *  PACKET RECEPTION CALLBACK (ISR CONTEXT)
 *
 *  Registered with the protocol parser at init.  Called whenever a
 *  complete, CRC-valid packet arrives.  Delegates to Command_Dispatch.
 * ========================================================================== */

static void OnPacketReceived(const PacketHeader *header,
                              const uint8_t *payload,
                              void *ctx)
{
    (void)ctx;
    Command_Dispatch(header, payload);
}

/* ==========================================================================
 *  COMMAND INTERFACE — INIT
 * ========================================================================== */

void CommandInterface_Init(void)
{
    ProtocolParser *p = CommDriver_GetParser();
    Protocol_ParserInit(p, OnPacketReceived, NULL);
}

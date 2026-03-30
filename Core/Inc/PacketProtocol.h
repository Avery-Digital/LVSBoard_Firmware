/*******************************************************************************
 * @file    Inc/PacketProtocol.h
 * @brief   Packet Protocol — Framing, Parsing, and Packet Building
 *
 *          Packet format (after byte-unstuffing):
 *          [SOF] [MSG1] [MSG2] [LEN_HI] [LEN_LO] [CMD1] [CMD2]
 *                [PAYLOAD ...] [CRC_HI] [CRC_LO] [EOF]
 *
 *          Byte stuffing:  If a data byte equals SOF, EOF, or ESC,
 *          transmit ESC followed by (byte ^ ESC).
 *
 *          CRC-16 CCITT is computed over the 6-byte header + payload
 *          (everything between SOF/EOF, after unstuffing, excluding CRC).
 *
 *          This module is transport-agnostic — it knows nothing about
 *          USART, DMA, or any hardware.
 *******************************************************************************
 * Copyright (c) 2026
 * All rights reserved.
 ******************************************************************************/

#ifndef PACKET_PROTOCOL_H
#define PACKET_PROTOCOL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "crc16.h"

/* =========================== Frame Constants ============================== */

#define FRAME_SOF           0x02U       /**< Start of frame marker           */
#define FRAME_EOF           0x7EU       /**< End of frame marker             */
#define FRAME_ESC           0x2DU       /**< Escape character                */

#define PKT_HEADER_SIZE     6U          /**< msg1+msg2+len_hi+len_lo+cmd1+cmd2 */
#define PKT_CRC_SIZE        2U          /**< CRC-16 = 2 bytes                */
#define PKT_MAX_PAYLOAD     4096U       /**< Maximum payload length          */

/** Worst-case TX buffer: SOF + every byte escaped (x2) + EOF */
#define PKT_TX_BUF_SIZE     (1U + (PKT_HEADER_SIZE + PKT_MAX_PAYLOAD + PKT_CRC_SIZE) * 2U + 1U)

/** RX assembly buffer: header + max payload + CRC */
#define PKT_RX_BUF_SIZE     (PKT_HEADER_SIZE + PKT_MAX_PAYLOAD + PKT_CRC_SIZE)

/* ============================ Packet Header =============================== */

typedef struct {
    uint8_t     msg1;
    uint8_t     msg2;
    uint16_t    length;         /**< Payload length (big-endian decoded)      */
    uint8_t     cmd1;
    uint8_t     cmd2;
} PacketHeader;

/* ========================== Parser State Machine ========================== */

typedef enum {
    PARSE_WAIT_SOF,
    PARSE_IN_FRAME,
    PARSE_ESCAPED,
} ParseState;

typedef void (*PacketRxCallback)(const PacketHeader *header,
                                 const uint8_t *payload,
                                 void *ctx);

typedef struct {
    /* State machine */
    ParseState      state;
    uint16_t        rx_index;
    uint16_t        expected_len;
    uint16_t        crc_rx;
    uint8_t         crc_bytes;

    /* Assembly buffer (unstuffed data: header + payload) */
    uint8_t         rx_buf[PKT_RX_BUF_SIZE];

    /* Decoded header (populated on valid packet) */
    PacketHeader    header;

    /* Application callback */
    PacketRxCallback  on_packet;
    void             *cb_ctx;

    /* Statistics */
    uint32_t        packets_ok;
    uint32_t        packets_err;
} ProtocolParser;

/* ============================ Public API ================================== */

void Protocol_ParserInit(ProtocolParser *parser,
                         PacketRxCallback callback,
                         void *ctx);

void Protocol_ParserReset(ProtocolParser *parser);

void Protocol_FeedBytes(ProtocolParser *parser,
                        const uint8_t *data, uint32_t len);

uint16_t Protocol_BuildPacket(uint8_t *tx_buf,
                              uint8_t msg1, uint8_t msg2,
                              uint8_t cmd1, uint8_t cmd2,
                              const uint8_t *payload, uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* PACKET_PROTOCOL_H */

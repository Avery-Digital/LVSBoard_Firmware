# LVS Board — Command Reference (v2.0.0)

Per-command request/response specification with worked byte-level examples.
LVS = Low-Voltage Switching board. Controls a 300-pin socket matrix (ADG714
analog switches) where each pin can be tied to one of several voltage rails
or left floating. Also runs pre-uploaded pin-state *patterns* through a
hardware sequencer.

**Protocol primer:**
- Frame: `SOF(0x02) [msg1][msg2][len_hi][len_lo][cmd1][cmd2][payload…] [CRC_hi][CRC_lo] EOF(0x7E)`
- `len` is `uint16` big-endian (payload length only).
- CRC: CRC-16 CCITT (poly `0x1021`, init `0xFFFF`) computed over `[msg1 … last payload byte]`.
- Byte stuffing: inside the frame, any `0x02 / 0x7E / 0x2D` byte is escaped as `ESC(0x2D) byte^0x2D`.
- All multi-byte integer fields on the wire are **big-endian** (v2.0.0).

All examples below use `msg1 = 0xA1`, `msg2 = 0xB2` (arbitrary — firmware
echoes them). Bytes are *pre-stuffing*; prepend `0x02` and append `0x7E`
and apply byte-stuffing to get the on-wire form.

Pin numbering: `0 … TOTAL_SOCKET_PINS - 1` where `TOTAL_SOCKET_PINS = 300`.

**Source encoding** (`source` byte):
- `0x00` = FLOAT (pin disconnected)
- `0x01` = VIN0 (first voltage rail)
- `0x02` = VIN1 (second voltage rail)

---

## Voltage Switcher bulk commands (`0x0E01 – 0x0E03`)

### `CMD_VS_ALL_FLOAT` — `0x0E01`
Floats every pin in the 300-pin socket matrix.

- Request: (no payload)
- Response (1 byte): `[0x00]` success, `[0xFF]` failure.

```
Request: A1 B2 00 00 0E 01 F3 0B
```

### `CMD_VS_ALL_VIN0` — `0x0E02`
Connects every pin to VIN0.

- Request: (none) / Response: `[0x00]` or `[0xFF]`.

```
Request: A1 B2 00 00 0E 02 C3 68
```

### `CMD_VS_ALL_VIN1` — `0x0E03`
Connects every pin to VIN1.

- Request: (none) / Response: `[0x00]` or `[0xFF]`.

```
Request: A1 B2 00 00 0E 03 D3 49
```

---

## Voltage Switcher single-pin (`0x0E04 – 0x0E05`)

### `CMD_VS_SET_SINGLE` — `0x0E04`
Route one pin to the requested source.

- Request (3 bytes): `[pin u16 BE][source]`
- Response (1 byte): `[0x00]` success / `[0xFF]` driver failure.
- Short payload (≠ 3 bytes): error response cmd = `0xF0 0x05`.

**Example — set pin 37 to VIN0:**
```
Request  : A1 B2 00 03 0E 04 00 25 00 ED 4E
                    len=3 cmd  pin=37 src=VIN0 CRC
Response : A1 B2 00 01 0E 04 00 5D 3D
                    len=1        ok
```

### `CMD_VS_GET_SINGLE` — `0x0E05`
Read the cached state of one pin.

- Request (2 bytes): `[pin u16 BE]`
- Response (3 bytes): `[pin_hi][pin_lo][source]` — pin echoed back in BE, followed by the current source byte.
- Error response cmd code: `0xFF 0x01` (driver read failed).

**Example — query pin 37, firmware reports VIN0:**
```
Request  : A1 B2 00 02 0E 05 00 25 1B 7D
Response : A1 B2 00 03 0E 05 00 25 00 9B FA
                              pin=37 src=VIN0
```

---

## Voltage Switcher all-pins (`0x0E06 – 0x0E07`)

### `CMD_VS_SET_ALL` — `0x0E06`
Set all 300 pins in one frame.

- Request (300 bytes, exactly `TOTAL_SOCKET_PINS`): one source byte per pin, indexed by pin number (0..299). Any size ≠ 300 triggers an error response.
- Response (1 byte): `[0x00]` ok / `[0xFF]` driver failure.

### `CMD_VS_GET_ALL` — `0x0E07`
Dump the current source byte for every pin.

- Request: (no payload)
- Response (300 bytes): one source byte per pin.

---

## Pattern sequencer (`0x0E10 – 0x0E13`)

Patterns are pre-uploaded 300-byte pin-state vectors stored in a slot table
(`PATTERN_TABLE_SLOTS = 1000`). A "run" is a sequence of `(slot, duration_ms)`
steps the hardware sequencer plays back.

### `CMD_SEQ_UPLOAD_PATTERN` — `0x0E10`
Write one pattern into a numbered slot.

- Request (302 bytes): `[slot u16 BE][pin_states 300 bytes]`
  - Any size other than `2 + PATTERN_PIN_COUNT` (= 302) triggers an error.
- Response (1 byte): `[0x00]` ok / `[0xFF]` write failed (slot index out of range).

### `CMD_SEQ_RUN` — `0x0E11`
Start playing a sequence of steps.

- Request: payload must be a multiple of 4 bytes, non-zero length. Each 4-byte group encodes one step — layout defined by the sequence table format (see `PatternTable.c`). Rough shape: `[slot u16 BE][duration u16 BE]` per step, but treat the sequence-table ABI as opaque and generate it via the host-side helper.
- `num_steps = header->length / 4`.
- Response (1 byte): `[0x00]` ok / `[0xFF]` if `Sequence_Start()` rejected the table.

### `CMD_SEQ_STOP` — `0x0E12`
Stop any running sequence.

- Request: (no payload) / Response: `[0x00]`.

```
Request: A1 B2 00 00 0E 12 D1 59
```

### `CMD_SEQ_STATUS` — `0x0E13`
Query sequencer state.

- Request: (no payload)
- Response (5 bytes): `[state][current_step u16 BE][total_steps u16 BE]`
  - `state`: implementation-defined enum from `Sequence_GetState()` (e.g. 0 idle, 1 running).

**Example — sequencer idle, no loaded sequence:**
```
Request  : A1 B2 00 00 0E 13 C1 78
Response : A1 B2 00 05 0E 13 00 00 00 00 00 16 E1
                          state step=0   total=0
```

---

## System / identity commands

### `CMD_BOARD_ID` — `0xE001`
- Request: (no payload)
- Response (3 bytes): ASCII `"LVS"` = `0x4C 0x56 0x53`.

```
Request  : A1 B2 00 00 E0 01 C0 B6
Response : A1 B2 00 03 E0 01 4C 56 53 FA FF
                          'L' 'V' 'S' CRC
```

### `CMD_FW_REV` — `0xE002`
Short hardware revision tag.

- Request: (no payload)
- Response (2 bytes): ASCII `"R1"` = `0x52 0x31`.

```
Request  : A1 B2 00 00 E0 02 F0 D5
Response : A1 B2 00 02 E0 02 52 31 82 35
```

### `CMD_BIST_STATUS` — `0xE0FF`
Boot self-test result. The result is latched at boot; this command just reports it.

- Request: (no payload)
- Response (4 bytes): ASCII `"PASS"` or `"FAIL"`.

```
Request         : A1 B2 00 00 E0 FF CE 67
Response (PASS) : A1 B2 00 04 E0 FF 50 41 53 53 A0 1F
```

### `CMD_GET_FW_VERSION` — `0x0E69`
Full firmware version string.

- Request: (no payload)
- Response: ASCII `"LVSBoard v2.0.0"` (15 bytes).

```
Request: A1 B2 00 00 0E 69 1E A5
```

### `CMD_PING` — `0xDEAD`
Echo. The request payload is copied back verbatim in the response — useful
for loopback testing the framing and CRC.

- Request: arbitrary payload (typically a few identifying bytes).
- Response: identical payload.

**Example — 3-byte echo:**
```
Request : A1 B2 00 03 DE AD 01 02 03 5B 8F
Response: A1 B2 00 03 DE AD 01 02 03 5B 8F
```

---

## Error response format

Short-payload or otherwise rejected commands produce a response with a
modified `cmd1/cmd2` pair:

- `0xF0 0x05` — payload length invalid (`SendError(..., 0xF0, 0x05)`)
- `0xFF 0x01` — execution failed (driver returned failure)

The response payload for these is typically a short explanatory byte; check
`CommandInterface.c` `SendError()` for the exact format per site.

---

## Migration note (v1.x → v2.0.0)

LVS wire format was already big-endian in v1.x — pin numbers, slot numbers,
step/total counters all packed MSB-first. v2.0.0 replaces the inline
`(b0 << 8) | b1` patterns with `be16_pack/unpack` from the shared
`endian_be.h` — byte-for-byte identical on the wire. The version bump is
a lockstep with the rest of the DMF firmware stack. The firmware version
string moved from `"LVSBoard v1.0.0"` to `"LVSBoard v2.0.0"`.

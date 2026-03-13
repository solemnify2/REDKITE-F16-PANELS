/*
  DcsBiosParser.h — DCS-BIOS binary frame parser for Teensy

  DCS-BIOS protocol:
    Sync: 0x55 0x55 0x55 0x55
    Then repeating: ADDR_LOW ADDR_HIGH COUNT_LOW COUNT_HIGH DATA[count]
    Frame end marker: address=0x5555, count=0x5555

  Only output addresses (LED control) are handled here.
  Switch input is handled via USB joystick (no DCS-BIOS write needed).
*/

#ifndef DCSBIOS_PARSER_H
#define DCSBIOS_PARSER_H

// ================================================================
//  DCS-BIOS F-16C Address Map
// ================================================================
// Source: DCS-BIOS F-16C_50.json
// These addresses may need verification against the actual JSON export.

// Address 0x447A: Nose gear (bit14), Left gear (bit15)
#define DCSBIOS_GEAR_NL_ADDR    0x447A
#define DCSBIOS_GEAR_NOSE_MASK  0x4000  // bit 14
#define DCSBIOS_GEAR_LEFT_MASK  0x8000  // bit 15

// Address 0x447C: Right gear (bit0)
#define DCSBIOS_GEAR_R_ADDR     0x447C
#define DCSBIOS_GEAR_RIGHT_MASK 0x0001  // bit 0

// Address 0x4544: ECM indicator (bit14)
#define DCSBIOS_ECM_ADDR        0x4544
#define DCSBIOS_ECM_MASK        0x4000  // bit 14

// ================================================================
//  Parser State Machine
// ================================================================

enum DcsBiosState {
  DCS_SYNC_1,
  DCS_SYNC_2,
  DCS_SYNC_3,
  DCS_SYNC_4,
  DCS_ADDR_LOW,
  DCS_ADDR_HIGH,
  DCS_COUNT_LOW,
  DCS_COUNT_HIGH,
  DCS_DATA
};

static DcsBiosState dcsBiosState = DCS_SYNC_1;
static uint16_t     dcsBiosAddr  = 0;
static uint16_t     dcsBiosCount = 0;
static uint16_t     dcsBiosSkip  = 0;
static uint8_t      dcsBiosDataBuf[2];
static uint8_t      dcsBiosDataIdx = 0;
static bool         dcsBiosCapture = false;  // true if current chunk is an address we care about

// ================================================================
//  Address Interest Check
// ================================================================

// Returns true if we need to capture data for this address
static bool dcsBiosIsInteresting(uint16_t addr) {
  return (addr == DCSBIOS_GEAR_NL_ADDR || addr == DCSBIOS_GEAR_R_ADDR || addr == DCSBIOS_ECM_ADDR);
}

// ================================================================
//  Update Callback — called when a watched address has new data
// ================================================================

// Forward-declared LED array and indices from main .ino
// (LedIdx enum and leds[] are defined in the main sketch)
static void dcsBiosOnUpdate(uint16_t addr, uint16_t value) {
  if (addr == DCSBIOS_GEAR_NL_ADDR) {
    writeLed(LI_NOSE_GEAR, (value & DCSBIOS_GEAR_NOSE_MASK));
    writeLed(LI_LEFT_GEAR, (value & DCSBIOS_GEAR_LEFT_MASK));
  }
  else if (addr == DCSBIOS_GEAR_R_ADDR) {
    writeLed(LI_RIGHT_GEAR, (value & DCSBIOS_GEAR_RIGHT_MASK));
  }
  else if (addr == DCSBIOS_ECM_ADDR) {
    writeLed(LI_ECM, (value & DCSBIOS_ECM_MASK));
  }
}

// ================================================================
//  Reset Parser State
// ================================================================

void dcsBiosReset() {
  dcsBiosState   = DCS_SYNC_1;
  dcsBiosAddr    = 0;
  dcsBiosCount   = 0;
  dcsBiosSkip    = 0;
  dcsBiosDataIdx = 0;
  dcsBiosCapture = false;
}

// ================================================================
//  Process One Byte
// ================================================================

void dcsBiosProcessByte(uint8_t b) {
  switch (dcsBiosState) {
    case DCS_SYNC_1:
      if (b == 0x55) dcsBiosState = DCS_SYNC_2;
      break;
    case DCS_SYNC_2:
      dcsBiosState = (b == 0x55) ? DCS_SYNC_3 : DCS_SYNC_1;
      break;
    case DCS_SYNC_3:
      dcsBiosState = (b == 0x55) ? DCS_SYNC_4 : DCS_SYNC_1;
      break;
    case DCS_SYNC_4:
      dcsBiosState = (b == 0x55) ? DCS_ADDR_LOW : DCS_SYNC_1;
      break;

    case DCS_ADDR_LOW:
      dcsBiosAddr = b;
      dcsBiosState = DCS_ADDR_HIGH;
      break;
    case DCS_ADDR_HIGH:
      dcsBiosAddr |= (uint16_t)b << 8;
      dcsBiosState = DCS_COUNT_LOW;
      break;
    case DCS_COUNT_LOW:
      dcsBiosCount = b;
      dcsBiosState = DCS_COUNT_HIGH;
      break;
    case DCS_COUNT_HIGH:
      dcsBiosCount |= (uint16_t)b << 8;

      // Frame end marker
      if (dcsBiosAddr == 0x5555 && dcsBiosCount == 0x5555) {
        dcsBiosState = DCS_SYNC_1;
        break;
      }

      if (dcsBiosCount == 0) {
        // Empty chunk, next address
        dcsBiosState = DCS_ADDR_LOW;
        break;
      }

      dcsBiosCapture = dcsBiosIsInteresting(dcsBiosAddr);
      dcsBiosDataIdx = 0;
      dcsBiosSkip    = 0;
      dcsBiosState   = DCS_DATA;
      break;

    case DCS_DATA:
      if (dcsBiosCapture && dcsBiosDataIdx < 2) {
        dcsBiosDataBuf[dcsBiosDataIdx] = b;
      }
      dcsBiosDataIdx++;
      dcsBiosSkip++;

      if (dcsBiosSkip >= dcsBiosCount) {
        // Chunk complete
        if (dcsBiosCapture && dcsBiosCount >= 2) {
          uint16_t value = dcsBiosDataBuf[0] | ((uint16_t)dcsBiosDataBuf[1] << 8);
          dcsBiosOnUpdate(dcsBiosAddr, value);
        }
        dcsBiosState = DCS_ADDR_LOW;  // Next chunk
      }
      break;
  }
}

#endif // DCSBIOS_PARSER_H

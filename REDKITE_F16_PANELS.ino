/* REDKITE_F16_PANELS
   Teensy 4.1 USB Joystick for Falcon BMS F-16 Left Console Panel

   USB Type: Keyboard + Mouse + Joystick (Tools > USB Type)

   Hardware configuration is data-driven.
   To add/remove/change hardware, edit the HARDWARE CONFIGURATION section only.
   Joystick button numbers are auto-assigned at runtime.
   To disable a panel, comment out its entries in the config arrays.
*/

// ================================================================
//  General Settings
// ================================================================

#define BAUDRATE      1000000
#define ALLOW_DEBUG   false
#define BUFFERSIZE    1024
#define MAX_PIN       55        // Teensy 4.1 max digital pin
#define LOOP_DELAY_MS 50
#define F4TS_TIMEOUT  6         // F4TS heartbeat timeout in seconds
#define MCP_SUPPORT   1         // 1 = route F4TS LightBit through MCP23017, 0 = F4TS direct only


// ================================================================
//  Type Definitions
// ================================================================

enum SwitchType { SW_ON_OFF, SW_ON_OFF_ON };

enum JoyAxis {
  AXIS_X, AXIS_Y, AXIS_Z,
  AXIS_Zr,                      // Zrotate
  AXIS_SL, AXIS_SR,             // sliderLeft/Right  (JOYSTICK_SIZE 12)
  AXIS_Xr, AXIS_Yr              // Xrotate/Yrotate   (JOYSTICK_SIZE 64)
};

enum Panel {
  PNL_NONE,       // standalone (not part of a panel)
  PNL_MISC,       // MISC Armament panel (center console, left of left MFD)
  PNL_GEAR,       // Landing Gear panel (left aux console)
  PNL_ALT_GEAR,   // Alt Gear panel (left aux console)
  PNL_CMDS,       // Countermeasures Dispenser (left aux console)
  PNL_TWA,        // Threat Warning Aux (left aux console)
  PNL_ECM,        // Electronic Countermeasures (left console)
  PNL_ELEC,       // Electrical panel (left console)
  PNL_EXT_LIGHT,  // External Lighting panel (left console)
  PNL_ICP,        // ICP / Instrument panel area
  PNL_COUNT
};

const char* const panelNames[] = {
  "Standalone", "MISC", "Gear", "AltGear", "CMDS", "TWA", "ECM", "ELEC", "ExtLight", "ICP"
};

struct SwitchDef {
  const char* name;
  Panel       panel;
  SwitchType  type;
  int8_t      mcpIdx;     // -1 = direct Teensy pin, >= 0 = index into mcpDevices[]
  uint8_t     pin1;
  uint8_t     pin2;       // only used for SW_ON_OFF_ON
  int*        stateRef;   // if non-NULL, switch state is written here
};

struct RotaryDef {
  const char*    name;
  Panel          panel;
  int8_t         mcpIdx;     // -1 = direct Teensy pin, >= 0 = index into mcpDevices[]
  uint8_t        numPos;     // number of positions mapped to joystick buttons
  uint8_t        numPins;    // number of physical pins (>= numPos)
  const uint8_t* pins;
};

// NOTE: PotDef and AnalogBtnArrayDef have no mcpIdx — MCP23017 does not support analog input.
struct PotDef {
  const char* name;
  Panel       panel;
  uint8_t     pin;
  JoyAxis     axis;
};

struct LedDef {
  const char* name;
  Panel       panel;
  uint8_t     pin;
  int8_t      mcpIdx;   // -1 = direct Teensy pin, >= 0 = index into mcpDevices[]
};

// --- MCP23017 I/O Expander ---

struct McpDeviceDef {
  const char* name;
  uint8_t     addr;     // I2C address (0x20–0x27)
};


// Resistor-ladder analog button array: multiple buttons on a single analog pin.
// Each button adds a 220Ω resistor, producing a unique analogRead value.
// Set 'values' to expected analogRead per button (ascending). 'tolerance' is +/- match window.
struct AnalogBtnArrayDef {
  const char*        groupName;    // group name for logging
  Panel              panel;
  uint8_t            pin;          // analog input pin
  uint8_t            numButtons;
  const char* const* btnNames;     // array of button names, length = numButtons
  const int*         values;       // expected analogRead per button, length = numButtons
  int                tolerance;    // +/- matching window
};

// ================================================================
//
//  >>> HARDWARE CONFIGURATION - Edit this section <<<
//
// ================================================================

// --- Switch state references ---
// Declare variables here for switches that need to be read by other logic.
// Then set stateRef in switches[] to link them.
int swLandingLight = 0;   // used by pedal brake mode
int swGear = 0;           // used by offline LED simulation

const SwitchDef switches[] = {
  // name                   panel           type         mcpIdx pin1 pin2  stateRef
  //                                                     (-1 = Teensy direct, >= 0 = MCP device index)

  // ---- MISC Panel (MCP23017 device 0, addr 0x20) ----
  // MCP pin mapping: GPA0–7 = 0–7, GPB0–7 = 8–15
  {"Laser ARM",             PNL_MISC,       SW_ON_OFF,       0,   0,   0,  NULL},   // GPA0
  {"RF",                    PNL_MISC,       SW_ON_OFF_ON,    0,   1,   2,  NULL},   // GPA1, GPA2
  {"Master ARM",            PNL_MISC,       SW_ON_OFF_ON,    0,   3,   4,  NULL},   // GPA3, GPA4
  {"Pitch AP",              PNL_MISC,       SW_ON_OFF_ON,    0,   5,   6,  NULL},   // GPA5, GPA6
  {"Roll AP",               PNL_MISC,       SW_ON_OFF_ON,    0,   7,   8,  NULL},   // GPA7, GPB0
  {"ALT REL",               PNL_MISC,       SW_ON_OFF,       0,   9,   0,  NULL},   // GPB1
  {"ADV MODE",              PNL_MISC,       SW_ON_OFF,       0,   10,   0,  NULL},   // GPB2

  // ---- Gear Panel (left aux console) ----
  {"EMER Jettison",         PNL_GEAR,       SW_ON_OFF,      -1,   10,   0,  NULL},
  {"Store CAT",             PNL_GEAR,       SW_ON_OFF,      -1,   5,   0,  NULL},   // CAT I / CAT III
  {"Horn Silencer",         PNL_GEAR,       SW_ON_OFF,      -1,   2,   0,  NULL},
  {"Landing Light",         PNL_GEAR,       SW_ON_OFF_ON,   -1,   3,   4,  &swLandingLight},
  {"Hook",                  PNL_GEAR,       SW_ON_OFF,      -1,   11,   0,  NULL},   // Up / Down
  {"Landing Gear",          PNL_GEAR,       SW_ON_OFF_ON,   -1,   28,   29,  &swGear},  // UP / DN
  {"DN LOCK REL",           PNL_GEAR,       SW_ON_OFF,      -1,   1,   0,  NULL},
  {"GND JETT ENABLE",       PNL_GEAR,       SW_ON_OFF,      -1,   9,   0,  NULL},   // ENABLE / OFF
  {"ANTI SKID",             PNL_GEAR,       SW_ON_OFF_ON,   -1,   6,  7,  NULL},   // PARKING BRAKE / ANTI SKID / OFF
  {"BRAKES Channel",        PNL_GEAR,       SW_ON_OFF,      -1,   8,   0,  NULL},   // CHAN 1 / CHAN 2

  // ---- CMDS Panel (left aux console) ----
  {"CMDS RWR",              PNL_CMDS,       SW_ON_OFF,      -1,  17,   0,  NULL},
  {"CMDS JMR",              PNL_CMDS,       SW_ON_OFF,      -1,  16,   0,  NULL},
  {"CMDS MWS",              PNL_CMDS,       SW_ON_OFF,      -1,  15,   0,  NULL},
  {"CMDS O1",               PNL_CMDS,       SW_ON_OFF,      -1,  20,   0,  NULL},
  {"CMDS O2",               PNL_CMDS,       SW_ON_OFF,      -1,  21,   0,  NULL},
  {"CMDS CH",               PNL_CMDS,       SW_ON_OFF,      -1,  22,   0,  NULL},
  {"CMDS FL",               PNL_CMDS,       SW_ON_OFF,      -1,  23,   0,  NULL},
  {"CMDS JETT",             PNL_CMDS,       SW_ON_OFF,      -1,  14,   0,  NULL},
// CMDS PRGM and CMDS MODE moved to analogBtnArrays[] (resistor ladder).

  // ---- Alt Gear Panel (left aux console) ----
  {"ALT GEAR Handle",       PNL_ALT_GEAR,   SW_ON_OFF,      -1,  33,   0,  NULL},
  // {"ALT GEAR Reset",        PNL_ALT_GEAR,   SW_ON_OFF,      -1,  15,   0,  NULL},

  // ---- ECM Panel (MCP23017 device 1, addr 0x21) ----
  // MCP pin mapping: GPA0–7 = 0–7, GPB0–7 = 8–15
  {"ECM XMIT",              PNL_ECM,        SW_ON_OFF_ON,    1,   0,   1,  NULL},   // GPA0, GPA1  (1 / 2 / 3)
  {"ECM RESET",             PNL_ECM,        SW_ON_OFF,       1,   2,   0,  NULL},   // GPA2
  {"ECM OPR",               PNL_ECM,        SW_ON_OFF_ON,    1,   3,   4,  NULL},   // GPA3, GPA4  (OPR / STBY / OFF)
  {"ECM BIT",               PNL_ECM,        SW_ON_OFF,       1,   5,   0,  NULL},   // GPA5
  // ECM 1~5, BLANK, FRM, SPL → moved to analogBtnArrays[] (resistor ladder)

  // ---- ELEC Panel (MCP23017 device 1, addr 0x21) ----
  {"MAIN PWR",              PNL_ELEC,       SW_ON_OFF_ON,    1,   6,   7,  NULL},   // GPA6, GPA7  (MAIN PWR / BATT / OFF)
  {"ELEC CAUTION RESET",    PNL_ELEC,       SW_ON_OFF,       1,   8,   0,  NULL},   // GPB0

  // ---- Standalone ----
  // {"Eject",                 PNL_NONE,       SW_ON_OFF,      -1,   0,   0,  NULL},
};

#define NUM_ROTARIES 0
const RotaryDef* const rotaries = nullptr;

// --- Analog Button Arrays (resistor ladder, multiple buttons on 1 analog pin) ---
// Each button adds 220Ω in series → unique analogRead value per button.
// values[] = expected analogRead per button. tolerance = +/- matching window.
// TODO: calibrate values[] by reading actual analogRead with ALLOW_DEBUG = true.

// Resistor ladder: 1kΩ series chain + 4.7kΩ pulldown per ladder.
// ADC_k = 1023 × 4700 / (k × 1000 + 4700).  Calibrate with ALLOW_DEBUG = true.

const char* const ecmBtnNames[] = {"ECM 1","ECM 2","ECM 3","ECM 4","ECM 5","ECM 6","ECM FRM","ECM SPL"};
const int         ecmBtnValues[] = {843, 718, 624, 553, 496, 449, 411, 379};

const char* const twaBtnNames[] = {"TWA SEARCH","TWA ACT/PWR","TWA ALT","TWA SYS PWR"};
const int         twaBtnValues[] = {839, 710, 615, 543};

const char* const cmdsModeBtnNames[] = {"MODE 1","MODE 2","MODE 3","MODE 4","MODE 5","MODE 6","MODE 7","MODE 8"};
const int         cmdsModeBtnValues[] = {843, 718, 624, 553, 496, 449, 411, 379};

const char* const cmdsPrgmBtnNames[] = {"PRGM BIT","PRGM 1","PRGM 2","PRGM 3","PRGM 4"};
const int         cmdsPrgmBtnValues[] = {843, 718, 624, 553, 496};

const AnalogBtnArrayDef analogBtnArrays[] = {
  // groupName       panel     pin  numBtn  btnNames           values              tolerance
  {"TWA Buttons",    PNL_TWA,  A10, 4,      twaBtnNames,       twaBtnValues,      30},
  // {"CMDS MODE",      PNL_CMDS, A11, 6,      cmdsModeBtnNames,  cmdsModeBtnValues, 15},
  // {"CMDS PRGM",      PNL_CMDS, A12, 5,      cmdsPrgmBtnNames,  cmdsPrgmBtnValues, 25},
  {"ECM Buttons",    PNL_ECM,  A13, 8,      ecmBtnNames,       ecmBtnValues,      15},
};

#define NUM_ANALOG_ARRAYS (sizeof(analogBtnArrays) / sizeof(analogBtnArrays[0]))


// --- Analog Pots ---
const PotDef pots[] = {
  // name      panel     pin   axis
//  {"ECM DIM",  PNL_ECM,  A14,  AXIS_SL},   // ECM brightness — requires JOYSTICK_SIZE 64 if ICP pots active
};

// --- LEDs ---
enum LedIdx { LI_NOSE_GEAR, LI_LEFT_GEAR, LI_RIGHT_GEAR, LI_TWA_POWER, LI_TWA_LOW, LI_TWA_SEARCH, LI_TWA_ACT, LI_ECM };

// pin 번호가 direct LED와 MCP LED에서 중복될 경우, MCP 쪽이 우선 적용됩니다.
// (mcpIdx >= 0 이면 MCP LED로 처리, -1 이면 direct LED로 처리)
const LedDef leds[] = {
  // name          panel      pin  mcpIdx
  {"Nose Gear",    PNL_GEAR,  30,  -1},
  {"Left Gear",    PNL_GEAR,  31,  -1},
  {"Right Gear",   PNL_GEAR,  32,  -1},
  {"TWA Power",    PNL_TWA,   34,  -1},
  {"TWA Low",      PNL_TWA,   35,  -1},
  {"TWA Search",   PNL_TWA,   36,  -1},
  {"TWA Act",      PNL_TWA,   37,  -1},
  {"ECM",          PNL_MISC,  11,   0},   // MCP device 0, MCP pin 11 (GPB3)
  };

// --- MCP23017 Devices ---
// Wire (SDA=18, SCL=19) — freed by moving CMDS MODE rotary to resistor ladder.
#define MCP_WIRE  Wire

const McpDeviceDef mcpDevices[] = {
  // name              addr
  {"MISC Panel",       0x20},
  {"ECM/ELEC Panel",   0x21},
};

// --- Pedal ---
#define PEDAL_ENABLED         true
#define PEDAL_PIN_LEFT        A16
#define PEDAL_PIN_RIGHT       A17
#define PEDAL_DEADZONE        300

#if JOYSTICK_SIZE == 12
  #define PEDAL_AXIS_LBRAKE   AXIS_SL
  #define PEDAL_AXIS_RBRAKE   AXIS_SR
#else
  #define PEDAL_AXIS_LBRAKE   AXIS_Xr
  #define PEDAL_AXIS_RBRAKE   AXIS_Yr
#endif
#define PEDAL_AXIS_RUDDER     AXIS_Zr

// Brake mode activates when this switch equals this value (or both pedals > deadzone)
// Pedal brake mode: activates when *PEDAL_BRAKE_SW_REF equals PEDAL_BRAKE_SW_VALUE
// Set PEDAL_BRAKE_SW_REF to NULL to disable switch-linked brake mode
#define PEDAL_BRAKE_SW_REF    (&swLandingLight)
#define PEDAL_BRAKE_SW_VALUE  (1)    // landing light UP position

// --- Offline LED ---
#define ECM_LED_IDX           LI_ECM

// ================================================================
//  End of Hardware Configuration
// ================================================================

#define NUM_SWITCHES      (sizeof(switches)      / sizeof(switches[0]))
#define NUM_MCP_DEVICES   (sizeof(mcpDevices)    / sizeof(mcpDevices[0]))
// NUM_ROTARIES defined above (0 — all rotaries converted to resistor ladders)
#define NUM_POTS          (sizeof(pots)          / sizeof(pots[0]))
#define NUM_LEDS          (sizeof(leds)          / sizeof(leds[0]))

// ================================================================
//  Includes
// ================================================================

#include <ArduinoJson.h>
#include <Keyboard.h>
#include <Wire.h>
#include <usb_dev.h>

extern volatile uint8_t usb_configuration;

#include "F4TS/MemoryFree.h"
#include "F4TS/F4ToSerialCommons.h"
#include "F4TS/F4ToSerialLightBits.h"

// ================================================================
//  MCP23017 I/O Expander Driver
// ================================================================

// Register addresses (IOCON.BANK = 0, default)
#define MCP_IODIRA  0x00
#define MCP_IODIRB  0x01
#define MCP_GPPUA   0x0C
#define MCP_GPPUB   0x0D
#define MCP_GPIOA   0x12
#define MCP_GPIOB   0x13
#define MCP_OLATA   0x14
#define MCP_OLATB   0x15

// Per-device state
static bool     mcpConnected[NUM_MCP_DEVICES];
static uint16_t mcpPortCache[NUM_MCP_DEVICES];
static uint8_t  mcpOutputA[NUM_MCP_DEVICES];
static uint8_t  mcpOutputB[NUM_MCP_DEVICES];

void mcpWriteReg(uint8_t addr, uint8_t reg, uint8_t val) {
  MCP_WIRE.beginTransmission(addr);
  MCP_WIRE.write(reg);
  MCP_WIRE.write(val);
  MCP_WIRE.endTransmission();
}

void mcpInit(uint8_t deviceIdx) {
  uint8_t addr = mcpDevices[deviceIdx].addr;

  // Probe: check if device responds on I2C bus
  MCP_WIRE.beginTransmission(addr);
  if (MCP_WIRE.endTransmission() != 0) {
    mcpConnected[deviceIdx] = false;
    mcpPortCache[deviceIdx] = 0xFFFF;  // all pins HIGH = all switches unpressed
    Serial.printf("  [MCP@0x%02X] %s — NOT DETECTED, skipping\n", addr, mcpDevices[deviceIdx].name);
    return;
  }
  mcpConnected[deviceIdx] = true;

  // Default: all pins input with pull-up
  uint8_t dirA = 0xFF, dirB = 0xFF;
  uint8_t pullA = 0xFF, pullB = 0xFF;

  // Set LED pins as output (no pull-up)
  for (unsigned int i = 0; i < NUM_LEDS; i++) {
    if (leds[i].mcpIdx == (int8_t)deviceIdx) {
      uint8_t p = leds[i].pin;
      if (p < 8) { dirA &= ~(1 << p); pullA &= ~(1 << p); }
      else       { dirB &= ~(1 << (p - 8)); pullB &= ~(1 << (p - 8)); }
    }
  }

  mcpWriteReg(addr, MCP_IODIRA, dirA);
  mcpWriteReg(addr, MCP_IODIRB, dirB);
  mcpWriteReg(addr, MCP_GPPUA,  pullA);
  mcpWriteReg(addr, MCP_GPPUB,  pullB);
  mcpWriteReg(addr, MCP_OLATA,  0x00);
  mcpWriteReg(addr, MCP_OLATB,  0x00);

  mcpOutputA[deviceIdx] = 0;
  mcpOutputB[deviceIdx] = 0;
  Serial.printf("  [MCP@0x%02X] %s — OK\n", addr, mcpDevices[deviceIdx].name);
}

// Read both ports and cache (skips disconnected devices)
// On I2C failure, keeps previous cache value (all switches unchanged).
void mcpReadPorts(uint8_t deviceIdx) {
  if (!mcpConnected[deviceIdx]) return;
  uint8_t addr = mcpDevices[deviceIdx].addr;
  MCP_WIRE.beginTransmission(addr);
  MCP_WIRE.write(MCP_GPIOA);
  if (MCP_WIRE.endTransmission() != 0) return;   // write failed — keep previous cache
  if (MCP_WIRE.requestFrom(addr, (uint8_t)2) != 2) return;  // read failed — keep previous cache
  uint8_t a = MCP_WIRE.read();
  uint8_t b = MCP_WIRE.read();
  mcpPortCache[deviceIdx] = a | ((uint16_t)b << 8);
}

// Read cached pin (call mcpReadPorts first)
bool mcpReadPin(uint8_t deviceIdx, uint8_t pin) {
  return (mcpPortCache[deviceIdx] >> pin) & 1;
}

// Write a single MCP output pin (skips disconnected devices)
void mcpWritePin(uint8_t deviceIdx, uint8_t pin, bool state) {
  if (!mcpConnected[deviceIdx]) return;
  uint8_t addr = mcpDevices[deviceIdx].addr;
  if (pin < 8) {
    if (state) mcpOutputA[deviceIdx] |= (1 << pin);
    else       mcpOutputA[deviceIdx] &= ~(1 << pin);
    mcpWriteReg(addr, MCP_OLATA, mcpOutputA[deviceIdx]);
  } else {
    uint8_t bit = pin - 8;
    if (state) mcpOutputB[deviceIdx] |= (1 << bit);
    else       mcpOutputB[deviceIdx] &= ~(1 << bit);
    mcpWriteReg(addr, MCP_OLATB, mcpOutputB[deviceIdx]);
  }
}

// Unified LED write — handles both direct and MCP LEDs
void writeLed(uint8_t idx, bool state) {
  if (leds[idx].mcpIdx >= 0) {
    mcpWritePin(leds[idx].mcpIdx, leds[idx].pin, state);
  } else {
    digitalWrite(leds[idx].pin, state ? HIGH : LOW);
  }
}

// ================================================================
//  F4TS LightBit with MCP23017 support
// ================================================================
//
// When MCP_SUPPORT is enabled, F4TS LightBit messages are intercepted
// here instead of being handled by the original F4TS parse functions.
//
// F4TS sends all LED pins in a single JSON message. Each pin is
// classified as either MCP (on MCP23017 I/O expander) or direct
// (Teensy GPIO), and routed accordingly:
//   - MCP pins  → writeLed() → mcpWritePin() via I2C
//   - Direct pins → add_lightBit() / digitalWriteFast() (same as F4TS original)
//
// When MCP_SUPPORT is disabled (0), the original F4TS functions
// (parse_setup_LightBit / parse_set_LightBit) are used as-is.
//
#if MCP_SUPPORT

// Per-pin routing table: maps each F4TS pin index to either
//   an MCP LED (ledIdx >= 0) or a direct F4TS LightBit (directIdx >= 0)
#define F4TS_PIN_MAP_MAX  32
struct F4tsPinRoute {
  int8_t ledIdx;      // >= 0: index in leds[] (MCP LED), -1: not MCP
  int8_t directIdx;   // >= 0: index in F4TS lightBits[] (direct), -1: not direct
};
static F4tsPinRoute f4tsPinRoute[F4TS_PIN_MAP_MAX];
static uint8_t      f4tsPinRouteCount = 0;

// Check if a pin belongs to an MCP LED in leds[]. Returns leds[] index or -1.
static int findMcpLedIdx(uint8_t pin) {
  for (unsigned int i = 0; i < NUM_LEDS; i++) {
    if (leds[i].pin == pin && leds[i].mcpIdx >= 0) return (int)i;
  }
  return -1;
}

// Clear MCP routing table (called alongside parse_destroy_lightBits)
void parse_destroy_LightBit_MCP_and_direct() {
  f4tsPinRouteCount = 0;
}

// Classify each pin as MCP or direct. Direct pins are registered
// with F4TS add_lightBit(); MCP pins are stored in the routing table.
void parse_setup_LightBit_MCP_and_direct(JsonVariant json) {
  if (!json.is<JsonObject>()) return;
  JsonArray pinsConfig = json.as<JsonObject>()["pins"].as<JsonArray>();
  int8_t directCount = 0;
  for (size_t i = 0; i < pinsConfig.size() && i < F4TS_PIN_MAP_MAX; i++) {
    uint8_t pin = (uint8_t)pinsConfig[i].as<int>();
    int mcpIdx = findMcpLedIdx(pin);
    f4tsPinRoute[i].ledIdx    = (mcpIdx >= 0) ? (int8_t)mcpIdx : -1;
    f4tsPinRoute[i].directIdx = (mcpIdx < 0)  ? directCount++  : -1;
    if (mcpIdx < 0) add_lightBit(pin);  // direct pin → F4TS manages
    f4tsPinRouteCount++;
  }
}

// Route each mode value per pin:
//   MCP pin    → writeLed() (I2C to MCP23017)
//   Direct pin → digitalWriteFast() (Teensy GPIO, via F4TS lightBits[])
void parse_set_LightBit_MCP_and_direct(JsonVariant json) {
  if (!json.is<JsonObject>()) return;
  JsonArray modes = json.as<JsonObject>()["mode"].as<JsonArray>();
  for (uint8_t i = 0; i < f4tsPinRouteCount && i < modes.size(); i++) {
    bool state = (modes[i].as<int>() != 0);
    if (f4tsPinRoute[i].ledIdx >= 0) {
      // MCP LED → I2C write
      writeLed((uint8_t)f4tsPinRoute[i].ledIdx, state);
    } else if (f4tsPinRoute[i].directIdx >= 0) {
      // Direct Teensy pin → fast GPIO write
      uint8_t di = (uint8_t)f4tsPinRoute[i].directIdx;
      if (di < LightBitCount && lightBits[di].mode != state) {
        lightBits[di].mode = state;
        digitalWriteFast(lightBits[di].pin, state ? HIGH : LOW);
      }
    }
  }
}

#endif // MCP_SUPPORT

#include "DcsBios/DcsBiosParser.h"

// ================================================================
//  Protocol Auto-Detection
// ================================================================

enum Protocol { PROTO_UNKNOWN, PROTO_F4TS, PROTO_DCSBIOS };

static Protocol  currentProto     = PROTO_UNKNOWN;
static uint8_t   syncCount        = 0;       // consecutive 0x55 bytes seen
static uint32_t  protoDetectStart = 0;       // millis() when first byte arrived
static int       onlineBlinkRemain = 0;      // remaining ON/OFF toggles for TWA1 blink
static int       onlineBlinkTimer  = 0;      // tick counter for 0.5s interval

// ================================================================
//  Global State
// ================================================================

// Runtime button mapping (auto-assigned in setup)
static uint8_t switchBtnStart[NUM_SWITCHES];
static uint8_t rotaryBtnStart[NUM_ROTARIES > 0 ? NUM_ROTARIES : 1];
static uint8_t analogBtnStart[NUM_ANALOG_ARRAYS];
static int     totalButtons = 0;

// ================================================================
//  Button Count Helper
// ================================================================

int switchButtonCount(SwitchType type) {
  switch (type) {
    case SW_ON_OFF:     return 1;
    case SW_ON_OFF_ON:  return 2;
  }
  return 1;
}

// Auto-assign joystick button numbers
void assignButtons() {
  int btn = 1;

  for (unsigned int i = 0; i < NUM_SWITCHES; i++) {
    switchBtnStart[i] = btn;
    btn += switchButtonCount(switches[i].type);
  }

  for (unsigned int i = 0; i < NUM_ROTARIES; i++) {
    rotaryBtnStart[i] = btn;
    btn += rotaries[i].numPos;
  }

  for (unsigned int i = 0; i < NUM_ANALOG_ARRAYS; i++) {
    analogBtnStart[i] = btn;
    btn += analogBtnArrays[i].numButtons;
  }

  totalButtons = btn - 1;

  // Check joystick button limit
  int maxButtons = (JOYSTICK_SIZE == 64) ? 128 : 32;
  if (totalButtons > maxButtons) {
    Serial.printf("ERROR: %d buttons assigned, but max is %d (JOYSTICK_SIZE=%d)\n",
      totalButtons, maxButtons, JOYSTICK_SIZE);
  }
}

// ================================================================
//  USB Suspend Detection (SOF-based, Teensy 4.x)
// ================================================================

#define USB_SUSPEND_THRESHOLD_MS  50

static uint32_t lastFrameIndex = 0;
static uint32_t lastSOFActiveTime = 0;

bool isUSBSuspended() {
  if (!usb_configuration) return true;

  uint32_t frame = USB1_FRINDEX;
  if (frame != lastFrameIndex) {
    lastFrameIndex = frame;
    lastSOFActiveTime = millis();
    return false;
  }
  return (millis() - lastSOFActiveTime > USB_SUSPEND_THRESHOLD_MS);
}

// ================================================================
//  Joystick Axis Helper
// ================================================================

void setJoystickAxis(JoyAxis axis, int rawValue) {
#if JOYSTICK_SIZE == 64
  int value = rawValue * 64;
#else
  int value = rawValue;
#endif
  switch (axis) {
    case AXIS_X:  Joystick.X(value);          break;
    case AXIS_Y:  Joystick.Y(value);          break;
    case AXIS_Z:  Joystick.Z(value);          break;
    case AXIS_Zr: Joystick.Zrotate(value);    break;
#if JOYSTICK_SIZE == 12
    case AXIS_SL: Joystick.sliderLeft(value);  break;
    case AXIS_SR: Joystick.sliderRight(value); break;
#endif
#if JOYSTICK_SIZE >= 64
    case AXIS_Xr: Joystick.Xrotate(value);    break;
    case AXIS_Yr: Joystick.Yrotate(value);     break;
#endif
    default: break;
  }
}

// ================================================================
//  Switch Processing
// ================================================================

// Helper: read a switch pin (direct or MCP)
inline int readSwPin(const SwitchDef& sw, uint8_t pin) {
  return (sw.mcpIdx >= 0) ? mcpReadPin(sw.mcpIdx, pin) : digitalRead(pin);
}

void processSwitches() {
  // Cache MCP ports before reading switches
  for (unsigned int d = 0; d < NUM_MCP_DEVICES; d++) {
    mcpReadPorts(d);
  }

  for (unsigned int i = 0; i < NUM_SWITCHES; i++) {
    int btn = switchBtnStart[i];
    const SwitchDef& sw = switches[i];

    switch (sw.type) {
      case SW_ON_OFF: {
        int state = !readSwPin(sw, sw.pin1);
        Joystick.button(btn, state);
        if (sw.stateRef) *sw.stateRef = state;
        break;
      }

      case SW_ON_OFF_ON: {
        int s1 = readSwPin(sw, sw.pin1);
        int s2 = readSwPin(sw, sw.pin2);
        Joystick.button(btn,     !s1);
        Joystick.button(btn + 1, !s2);
        if (sw.stateRef) *sw.stateRef = -(!s1) + (!s2);   // -1 = dn, 0 = center, 1 = up
        break;
      }
    }
  }
}

// ================================================================
//  Rotary Processing
// ================================================================

void processRotaries() {
  for (unsigned int r = 0; r < NUM_ROTARIES; r++) {
    int btn = rotaryBtnStart[r];

    const RotaryDef& rot = rotaries[r];
    for (int i = 0; i < rot.numPos; i++) {
      bool pinState = (rot.mcpIdx >= 0)
        ? mcpReadPin(rot.mcpIdx, rot.pins[i])
        : digitalRead(rot.pins[i]);
      Joystick.button(btn + i, !pinState);
    }
  }
}

// ================================================================
//  Analog Button Array Processing (resistor ladder)
// ================================================================

void processAnalogButtons() {
  for (unsigned int a = 0; a < NUM_ANALOG_ARRAYS; a++) {
    int btn = analogBtnStart[a];

    const AnalogBtnArrayDef& arr = analogBtnArrays[a];
    int raw = analogRead(arr.pin);

    for (int i = 0; i < arr.numButtons; i++) {
      bool matched = (raw >= arr.values[i] - arr.tolerance) &&
                     (raw <= arr.values[i] + arr.tolerance);
      Joystick.button(btn + i, matched);
    }

    // if (ALLOW_DEBUG) {
    if ( raw > 10 ) 
      Serial.printf("[%s] raw=%d\n", arr.groupName, raw);
    // }
  }
}

// ================================================================
//  Pot Processing
// ================================================================

void processPots() {
  for (unsigned int i = 0; i < NUM_POTS; i++) {
    setJoystickAxis(pots[i].axis, analogRead(pots[i].pin));
  }
}

// ================================================================
//  Pedal Processing (brake + rudder synthesis)
// ================================================================

void processPedal() {
#if PEDAL_ENABLED
  static int lbMin = 300, lbMax = 750;
  static int rbMin = 300, rbMax = 750;

  int lb = analogRead(PEDAL_PIN_LEFT);
  int rb = analogRead(PEDAL_PIN_RIGHT);

  // Auto calibration
  lbMin = min(lbMin, lb);  lbMax = max(lbMax, lb);
  rbMin = min(rbMin, rb);  rbMax = max(rbMax, rb);

  lb = (lb - lbMin) * 1024 / (lbMax - lbMin);
  rb = (rb - rbMin) * 1024 / (rbMax - rbMin);

  // Brake mode: linked switch active OR both pedals beyond deadzone
  bool brakeMode = (lb > PEDAL_DEADZONE && rb > PEDAL_DEADZONE);
  if (PEDAL_BRAKE_SW_REF)
    brakeMode = brakeMode || (*PEDAL_BRAKE_SW_REF == PEDAL_BRAKE_SW_VALUE);

  setJoystickAxis(PEDAL_AXIS_LBRAKE, brakeMode ? lb : 0);
  setJoystickAxis(PEDAL_AXIS_RBRAKE, brakeMode ? rb : 0);
  setJoystickAxis(PEDAL_AXIS_RUDDER, brakeMode ? (1024 / 2) : ((1023 - lb + rb) / 2));
#endif
}

// ================================================================
//  LED Control
// ================================================================

void turnOffAllLeds() {
  for (unsigned int i = 0; i < NUM_LEDS; i++) {
    writeLed(i, false);
  }

#ifdef USE_LIGHTBITS
  for (int i = 0; i < LightBitCount; i++) {
    digitalWriteFast(lightBits[i].pin, LOW);
    lightBits[i].mode = false;
  }
#endif
}

void updateLedsOffline() {
  static int blinkCounter = 0;
  const int ticksPerSecond = 1000 / LOOP_DELAY_MS;

  // Gear LEDs: cycle through 3 LEDs, 1 second each
  static int gearPhase = 0;


  for (unsigned int i = 0; i < NUM_LEDS; i++) {
    if ((int)i == ECM_LED_IDX)
      writeLed(i, blinkCounter / (ticksPerSecond / 2));
    else if (i >= LI_NOSE_GEAR && i <= LI_RIGHT_GEAR)
      writeLed(i, (int)(i - LI_NOSE_GEAR) == gearPhase ? 1 : 0);
    else if (i >= LI_TWA_POWER && i <= LI_TWA_ACT)
      writeLed(i, 0);
    else
      writeLed(i, 0);
  }

  blinkCounter = (blinkCounter + 1) % ticksPerSecond;

  if (blinkCounter == 0) {
    gearPhase = (gearPhase + 1) % 3;
  }
}

// ================================================================
//  F4TS Serial Communication
// ================================================================

/*
int readSerialLine(int readch, char *buffer, int len) {
  static int pos = 0;
  int rpos;

  if (readch > 0) {
    switch (readch) {
      case '\n': break;
      case '\r':
        rpos = pos;
        pos = 0;
        return rpos;
      default:
        if (pos < len - 1) {
          buffer[pos++] = readch;
          buffer[pos] = 0;
        }
    }
    return 0;
  }
  return -1;
}
*/

// Process one byte through the F4TS JSON readline parser.
// Returns: >0 = complete JSON processed, 0 = partial, -1 = no data
int processF4TSByte(int ch) {
  static char buffer[BUFFERSIZE];
  int ret = readline(ch, buffer, BUFFERSIZE);

  if (ret <= 0) return ret;

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, buffer);
  if (error) {
    if (ALLOW_DEBUG) { Serial.print("Deserialization Error: "); Serial.println(error.c_str()); }
    return 0;
  }

  ret = 1;

#ifdef USE_MOTORS
  if (doc["setup_stepper"].is<JsonVariant>())  parse_setup_command(doc["setup_stepper"]);
  if (doc["setstep"].is<JsonVariant>())      { parse_setstep_command(doc["setstep"]); ret = 2; }
#endif

#ifdef USE_BCD
  if (doc["setup_bcd"].is<JsonVariant>() && doc["digitOn"].is<JsonVariant>() && doc["digitOff"].is<JsonVariant>())
    parse_setup_bcd(doc);
  if (doc["set_seg"].is<JsonVariant>())        { parse_set_seg_command(doc["set_seg"]); ret = 3; }
  if (doc["destroy_all_bcd"].is<JsonVariant>()) parse_destroy_bcd();
#endif

#ifdef USE_OLED_DISPLAY
  if (doc["setup_display"].is<JsonVariant>())       parse_setup_display(doc["setup_display"]);
  else if (doc["set_display"].is<JsonVariant>())  { parse_set_display(doc["set_display"]); ret = 4; }
#endif

#ifdef USE_LIGHTBITS
  if (doc["setup_LightBit"].is<JsonVariant>()) {
    parse_destroy_lightBits();                           // clear F4TS lightBits[] (common)
  #if MCP_SUPPORT
    parse_destroy_LightBit_MCP_and_direct();             // clear MCP routing table
    parse_setup_LightBit_MCP_and_direct(doc["setup_LightBit"]);  // classify & register all pins
  #else
    parse_setup_LightBit(doc["setup_LightBit"]);         // F4TS original (direct only)
  #endif
  }
  else if (doc["set_LightBit"].is<JsonVariant>()) {
  #if MCP_SUPPORT
    parse_set_LightBit_MCP_and_direct(doc["set_LightBit"]);  // route MCP & direct per-pin
  #else
    parse_set_LightBit(doc["set_LightBit"]);                  // F4TS original (direct only)
  #endif
    ret = 5;
  }
#endif

#ifdef USE_MATRIX
  if (doc["setup_matrix"].is<JsonVariant>())        parse_setup_matrice(doc["setup_matrix"]);
  else if (doc["destroy_matrix"].is<JsonVariant>()) { parse_destroy_matrix(); ret = 6; }
  else if (doc["update_matrix"].is<JsonVariant>())  parse_set_matrice(doc["update_matrix"]);
#endif

  if (ALLOW_DEBUG) { Serial.print("Free Memory = "); Serial.println(freeMemory()); }

  return ret;
}

// ================================================================
//  Protocol Detection & Serial Routing
// ================================================================

void resetProtocol() {
  currentProto = PROTO_UNKNOWN;
  syncCount    = 0;
  protoDetectStart = 0;
  resetReadline();
  dcsBiosReset();
  turnOffAllLeds();
  if (ALLOW_DEBUG) Serial.println("[Proto] Reset to UNKNOWN");
}

// Read all available serial bytes, auto-detect protocol, and route.
// Returns true if any data was received (heartbeat reset).
bool detectAndRouteSerial() {
  bool received = false;

  while (Serial.available()) {
    int ch = Serial.read();
    if (ch < 0) break;
    received = true;

    switch (currentProto) {

      case PROTO_UNKNOWN: {
        // Start detection timer on first byte
        if (protoDetectStart == 0) protoDetectStart = millis();

        uint8_t b = (uint8_t)ch;
        if (b == 0x55) {
          syncCount++;
          if (syncCount >= 4) {
            currentProto = PROTO_DCSBIOS;
            syncCount = 0;
            writeLed(LI_TWA_LOW, 1);
            onlineBlinkRemain = 8;  // 4 blinks (ON/OFF × 4)
            onlineBlinkTimer = 0;
            if (ALLOW_DEBUG) Serial.println("[Proto] Detected DCS-BIOS");
            // The 4 sync bytes are consumed; parser starts at ADDR_LOW
            dcsBiosReset();
            dcsBiosState = DCS_ADDR_LOW;  // already past sync
          }
        } else {
          syncCount = 0;
          // Printable ASCII or JSON start → F4TS
          if (b >= 0x20 && b < 0x7F) {
            currentProto = PROTO_F4TS;
            writeLed(LI_TWA_POWER, 1);
            onlineBlinkRemain = 8;  // 4 blinks (ON/OFF × 4)
            onlineBlinkTimer = 0;
            if (ALLOW_DEBUG) Serial.println("[Proto] Detected F4TS");
            // Feed this first byte to F4TS
            processF4TSByte(ch);
          }
          // else: non-printable, non-0x55 → ignore, keep detecting
        }
        break;
      }

      case PROTO_F4TS:
        processF4TSByte(ch);
        break;

      case PROTO_DCSBIOS:
        dcsBiosProcessByte((uint8_t)ch);
        break;
    }
  }

  return received;
}

// ================================================================
//  Setup
// ================================================================

void setup() {
  Serial.begin(BAUDRATE);
  Joystick.useManualSend(true);

#if JOYSTICK_SIZE == 12
  Joystick.hat(-1);
#elif JOYSTICK_SIZE == 64
  Joystick.hat(0, -1);
#endif

  // Default all pins to OUTPUT (reduce power)
  for (int i = 0; i < MAX_PIN; i++)
    pinMode(i, OUTPUT);

  // Configure switch pins (direct only; MCP switches are configured by mcpInit)
  for (unsigned int i = 0; i < NUM_SWITCHES; i++) {
    if (switches[i].mcpIdx >= 0) continue;
    pinMode(switches[i].pin1, INPUT_PULLUP);
    if (switches[i].type == SW_ON_OFF_ON)
      pinMode(switches[i].pin2, INPUT_PULLUP);
  }

  // Configure rotary pins (direct only; MCP rotaries are configured by mcpInit)
  for (unsigned int i = 0; i < NUM_ROTARIES; i++) {
    if (rotaries[i].mcpIdx >= 0) continue;
    for (int j = 0; j < rotaries[i].numPins; j++)
      pinMode(rotaries[i].pins[j], INPUT_PULLUP);
  }

  // Configure analog button array pins
  for (unsigned int i = 0; i < NUM_ANALOG_ARRAYS; i++) {
    pinMode(analogBtnArrays[i].pin, INPUT);
  }

  // Configure pot pins
  for (unsigned int i = 0; i < NUM_POTS; i++) {
    pinMode(pots[i].pin, INPUT);
  }

  // Configure LED pins (direct only; MCP LEDs are configured by mcpInit)
  for (unsigned int i = 0; i < NUM_LEDS; i++) {
    if (leds[i].mcpIdx < 0)
      pinMode(leds[i].pin, OUTPUT);
  }

  // Initialize MCP23017 I/O expanders
  MCP_WIRE.begin();
  for (unsigned int i = 0; i < NUM_MCP_DEVICES; i++) {
    mcpInit(i);
  }

  // Configure pedal pins
#if PEDAL_ENABLED
  pinMode(PEDAL_PIN_LEFT,  INPUT);
  pinMode(PEDAL_PIN_RIGHT, INPUT);
#endif

  // Auto-assign joystick button numbers
  assignButtons();

  // Print configuration summary
  Serial.println("=== REDKITE_F16_PANELS ===");
  for (int p = 0; p < PNL_COUNT; p++) {
    int nSw = 0, nRot = 0, nPot = 0, nLed = 0, nAna = 0;
    for (unsigned int i = 0; i < NUM_SWITCHES;      i++) if (switches[i].panel        == p) nSw++;
    for (unsigned int i = 0; i < NUM_ROTARIES;      i++) if (rotaries[i].panel        == p) nRot++;
    for (unsigned int i = 0; i < NUM_ANALOG_ARRAYS; i++) if (analogBtnArrays[i].panel == p) nAna++;
    for (unsigned int i = 0; i < NUM_POTS;          i++) if (pots[i].panel            == p) nPot++;
    for (unsigned int i = 0; i < NUM_LEDS;          i++) if (leds[i].panel            == p) nLed++;
    if (nSw + nRot + nAna + nPot + nLed == 0) continue;
    Serial.printf("  [%s] sw:%d rot:%d ana:%d pot:%d led:%d\n",
      panelNames[p], nSw, nRot, nAna, nPot, nLed);
  }
  Serial.printf("  Total buttons: %d\n", totalButtons);
  Serial.println("=========================");

  // Print button mapping detail
  for (unsigned int i = 0; i < NUM_SWITCHES; i++) {
    int cnt = switchButtonCount(switches[i].type);
    if (switches[i].mcpIdx >= 0) {
      if (cnt == 1)
        Serial.printf("  btn %d      : %s [MCP@0x%02X]\n", switchBtnStart[i], switches[i].name, mcpDevices[switches[i].mcpIdx].addr);
      else
        Serial.printf("  btn %d~%d   : %s [MCP@0x%02X]\n", switchBtnStart[i], switchBtnStart[i] + cnt - 1, switches[i].name, mcpDevices[switches[i].mcpIdx].addr);
    } else {
      if (cnt == 1)
        Serial.printf("  btn %d      : %s\n", switchBtnStart[i], switches[i].name);
      else
        Serial.printf("  btn %d~%d   : %s\n", switchBtnStart[i], switchBtnStart[i] + cnt - 1, switches[i].name);
    }
  }
  for (unsigned int i = 0; i < NUM_ROTARIES; i++) {
    Serial.printf("  btn %d~%d   : %s (%d-pos)\n",
      rotaryBtnStart[i], rotaryBtnStart[i] + rotaries[i].numPos - 1, rotaries[i].name, rotaries[i].numPos);
  }
  for (unsigned int i = 0; i < NUM_ANALOG_ARRAYS; i++) {
    Serial.printf("  btn %d~%d   : %s (%d-btn analog ladder)\n",
      analogBtnStart[i], analogBtnStart[i] + analogBtnArrays[i].numButtons - 1,
      analogBtnArrays[i].groupName, analogBtnArrays[i].numButtons);
  }
  Serial.println("=========================");
}

// ================================================================
//  Main Loop
// ================================================================

void loop() {
  if (isUSBSuspended()) {
    turnOffAllLeds();
    asm("wfi");   // CPU sleep until next interrupt (USB resume, timer, etc.)
    return;
  }

  // --- Read all inputs ---
  processSwitches();
  processRotaries();
  processAnalogButtons();
  processPots();
  processPedal();

  // --- Send joystick state ---
  Joystick.send_now();

  // --- Serial communication & LED control ---
  static int heartbeat = 0;
  const int timeoutTicks = (1000 / LOOP_DELAY_MS) * F4TS_TIMEOUT;

  if (detectAndRouteSerial()) {
    heartbeat = 0;
  }

  if (heartbeat >= timeoutTicks) {
    // Timeout: reset protocol so we can re-detect after sim switch
    if (currentProto != PROTO_UNKNOWN) {
      resetProtocol();
    }
    updateLedsOffline();
  }

  ++heartbeat;
  heartbeat = min(heartbeat, timeoutTicks);

  // Online detection blink: TWA1 blinks 4 times at 0.5s interval
  if (onlineBlinkRemain > 0) {
    const int halfSecTicks = 500 / LOOP_DELAY_MS;  // 10 ticks
    onlineBlinkTimer++;
    if (onlineBlinkTimer >= halfSecTicks) {
      onlineBlinkTimer = 0;
      onlineBlinkRemain--;
      writeLed(LI_TWA_POWER, onlineBlinkRemain & 1);  // odd=ON, even=OFF
    }
  }

  delay(LOOP_DELAY_MS);
}

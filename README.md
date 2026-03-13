# REDKITE F16 PANELS

Teensy 4.1 기반 F-16 콕핏 패널 컨트롤러.
**Falcon BMS**(F4ToSerial)와 **DCS World**(DCS-BIOS) 듀얼 심 지원, 프로토콜 자동 감지.

## Features

- **USB Joystick + Keyboard** 복합 디바이스 (Windows 자동 인식)
- **데이터 드리븐 설계** — 스위치/로터리/LED 등 하드웨어 변경 시 config 배열만 수정
- **조이스틱 버튼 자동 할당** — 런타임에 순차 번호 부여, 수동 매핑 불필요
- **MCP23017 I2C 확장** — I/O 핀 부족 시 I2C로 확장 가능
- **저항 래더 입력** — 아날로그 핀 1개로 다중 버튼 인식 (로터리 스위치 대체)
- **페달 처리** — 러더 + 독립 브레이크 합성, 자동 캘리브레이션
- **USB Suspend 감지** — PC 절전 시 LED 소등 및 MCU 저전력 모드 진입
- **프로토콜 자동 감지** — F4TS(JSON) / DCS-BIOS(바이너리) 자동 판별

## Supported Panels

| Panel | Type | Description |
|-------|------|-------------|
| Gear | Switch, LED | Landing Gear, Hook, Anti-skid, Landing Light 등 |
| Alt Gear | Switch | ALT Gear Handle / Reset |
| MISC | Switch, LED (MCP23017) | Master ARM, Laser ARM, RF, Autopilot Pitch/Roll 등 |
| CMDS | Switch, Analog Ladder | RWR, JMR, MWS, JETT, MODE, PRGM 등 |
| TWA | Analog Ladder | Threat Warning Aux — Search, Act/Pwr, Alt, Sys Pwr |
| ECM | Switch, Analog Ladder, Pot | OPR, XMIT, Reset, BIT, ECM 1~6, FRM, SPL, DIM |
| ELEC | Switch | Main PWR, Caution Reset |
| Pedal | Analog | Rudder + Left/Right Brake (auto calibration) |

## Hardware

### Requirements

- **Teensy 4.1**
- MCP23017 I2C I/O Expander (MISC 패널용, 선택)
- 저항 래더 회로 (1kΩ 직렬 + 4.7kΩ 풀다운, 아날로그 버튼용)
- 외부 풀업 저항 4.7kΩ (I2C SDA/SCL, MCP23017 사용 시 권장)

### Pin Map

| Pin | Function |
|-----|----------|
| 0 | Eject |
| 1–15 | Gear / Alt Gear 패널 스위치 |
| 16, 17, 20 | Gear LED (Nose, Left, Right) |
| 18 (SDA), 19 (SCL) | I2C — MCP23017 |
| 21–37 | CMDS, ECM, ELEC 패널 스위치 |
| A10, A11 | Pedal (Left, Right) |
| A12 | TWA 저항 래더 |
| A13 | ECM 저항 래더 |
| A14 | ECM DIM 포텐셔미터 |
| A15 | CMDS MODE 저항 래더 |
| A16 | CMDS PRGM 저항 래더 |

## Build

### Arduino IDE Setup

1. [Teensyduino](https://www.pjrc.com/teensy/td_download.html) 설치
2. Arduino IDE에서:
   - **Board**: Teensy 4.1
   - **USB Type**: Keyboard + Mouse + Joystick
3. 필요 라이브러리:
   - `ArduinoJson`
   - `Wire` (built-in)
   - `Keyboard` (built-in)

### Upload

`REDKITE_F16_PANELS.ino`를 열고 Upload.

## Configuration

하드웨어 변경은 `REDKITE_F16_PANELS.ino`의 **HARDWARE CONFIGURATION** 섹션만 수정합니다.

```cpp
// 스위치 추가 예시
const SwitchDef switches[] = {
  // name         panel        type         mcpIdx  pin1  pin2  stateRef
  {"My Switch",   PNL_GEAR,    SW_ON_OFF,      -1,    5,    0,  NULL},
};

// 아날로그 버튼 배열 추가 예시
const AnalogBtnArrayDef analogBtnArrays[] = {
  // groupName    panel      pin  numBtn  btnNames       values         tolerance
  {"My Buttons",  PNL_ECM,   A13, 4,      myBtnNames,    myBtnValues,   20},
};
```

패널을 비활성화하려면 해당 항목을 주석 처리하면 됩니다.

## Protocol Support

### Falcon BMS (F4ToSerial)

- JSON 기반 시리얼 통신 (1Mbaud)
- Stepper Motors, BCD 디스플레이, OLED, LightBit, Matrix LED 지원
- F4ToSerial PC 프로그램 필요

### DCS World (DCS-BIOS)

- 바이너리 프로토콜, `0x55` sync 4바이트로 자동 감지
- DCS-BIOS 설치 및 설정 필요

시뮬레이터 전환 시 6초 타임아웃 후 자동 재감지.

## Project Structure

```
REDKITE_F16_PANELS/
├── REDKITE_F16_PANELS.ino   # 메인 프로그램
├── name.c                    # USB 디바이스명 설정
├── F4TS/                     # F4ToSerial 라이브러리
│   ├── F4ToSerial.ino
│   ├── F4ToSerialCommons.h
│   ├── F4ToSerialLightBits.h
│   ├── F4ToSerialBCD.h
│   ├── F4ToSerialMotors.h
│   ├── F4ToSerialOledDisplays.h
│   ├── F4ToSerialMatrixLightBits.h
│   ├── F4ToSerialFont_ded.h
│   ├── F4ToSerialFont_ffi.h
│   ├── F4ToserialFont_pfl.h
│   ├── MemoryFree.cpp / .h
│   └── digitalWriteFast.h
├── DcsBios/
│   └── DcsBiosParser.h       # DCS-BIOS 프로토콜 파서
└── backup/                    # 이전 버전 아카이브
```

## License

This project is licensed under the [MIT License](LICENSE).

**Note:** The `F4TS/` directory contains code from the [F4ToSerial](https://github.com/Music-Junky/F4ToSerial) project by Music-Junky. This code is not covered by the MIT License of this repository and is subject to its original author's terms. Please refer to the F4ToSerial project for its licensing information.

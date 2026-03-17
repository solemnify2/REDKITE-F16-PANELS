# REDKITE F16 PANELS

Teensy 4.1 기반 F-16 콕핏 패널 컨트롤러.
**Falcon BMS**(BMS-BIOS)와 **DCS World**(DCS-BIOS) 듀얼 심 지원, 프로토콜 자동 감지.

## Features

- **USB Joystick + Keyboard** 복합 디바이스 (Windows 자동 인식)
- **데이터 드리븐 설계** — 스위치/LED 등 하드웨어 변경 시 config 배열만 수정
- **조이스틱 버튼 자동 할당** — 런타임에 순차 번호 부여, 수동 매핑 불필요
- **MCP23017 I2C 확장** — I/O 핀 부족 시 I2C로 확장 가능
- **저항 래더 입력** — 아날로그 핀 1개로 다중 버튼 인식 (로터리 스위치 대체)
- **페달 처리** — 러더 + 독립 브레이크 합성, 자동 캘리브레이션
- **USB Suspend 감지** — PC 절전 시 LED 소등 및 MCU 저전력 모드 진입
- **프로토콜 자동 감지** — BMS-BIOS(바이너리) / DCS-BIOS(바이너리) 자동 판별

## Supported Panels

| Panel | Type | Description |
|-------|------|-------------|
| Gear | Switch, LED | Landing Gear, Hook, Anti-skid, Landing Light, Gear Warning 등 |
| Alt Gear | Switch | ALT Gear Handle / Reset |
| MISC | Switch, LED (MCP23017) | Master ARM, Laser ARM, RF, Autopilot Pitch/Roll, ECM LED 등 |
| CMDS | Switch, Analog Ladder | RWR, JMR, MWS, JETT, MODE (6-pos), PRGM (5-pos) 등 |
| TWA | Analog Ladder, LED | Threat Warning Aux — Search, Act/Pwr, Low, Power |
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
| 1–11 | Gear 패널 스위치 |
| 12, 13 | Alt Gear Handle / Reset |
| 14–17 | CMDS 패널 스위치 |
| 18 (SDA), 19 (SCL) | I2C — MCP23017 |
| 20–23 | CMDS 패널 스위치 |
| 24 (A10) | TWA 저항 래더 (4 버튼) |
| 25 (A11) | CMDS MODE 저항 래더 (6 버튼) |
| 26 (A12) | CMDS PRGM 저항 래더 (5 버튼) |
| 28, 29 | Landing Gear 스위치 |
| 30–33 | Gear LED (Nose, Left, Right, Warning) |
| 34–37 | TWA LED (Power, Low, Search, Act) |
| 40 (A16), 41 (A17) | Pedal (Left, Right Brake) |

미사용 핀: 0, 27(A13), 38(A14), 39(A15)

## Build

### Arduino IDE Setup

1. [Teensyduino](https://www.pjrc.com/teensy/td_download.html) 설치
2. Arduino IDE에서:
   - **Board**: Teensy 4.1
   - **USB Type**: Keyboard + Mouse + Joystick
3. 필요 라이브러리:
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
  {"My Buttons",  PNL_CMDS,  A11, 4,      myBtnNames,    myBtnValues,   20},
};
```

패널을 비활성화하려면 해당 항목을 주석 처리하면 됩니다.

## Protocol Support

### Falcon BMS (BMS-BIOS)

- 바이너리 프로토콜, `0xAA 0xBB` sync 2바이트로 자동 감지
- Python 브릿지(`tools/bmsbios_bridge.py`)가 BMS 공유 메모리에서 LED 상태를 읽어 Teensy로 전송
- BMS 실행 전에 bridge를 먼저 실행해도 안전 (공유 메모리 생성하지 않음)
- 사용법:
  ```
  pip install pyserial
  python tools/bmsbios_bridge.py COM4
  python tools/bmsbios_bridge.py COM4 --hz 20 --debug
  ```

### DCS World (DCS-BIOS)

- 바이너리 프로토콜, `0x55` sync 4바이트로 자동 감지
- DCS-BIOS 설치 및 UDP 브릿지(`tools/dcsbios_bridge.py`) 필요
- 사용법:
  ```
  pip install pyserial
  python tools/dcsbios_bridge.py COM4
  ```

시뮬레이터 전환 시 6초 타임아웃 후 자동 재감지.

## Project Structure

```
REDKITE_F16_PANELS/
├── REDKITE_F16_PANELS.ino    # 메인 프로그램
├── name.c                     # USB 디바이스명 설정
├── BiosHandler/
│   ├── DcsBiosParser.h        # DCS-BIOS 프로토콜 파서
│   └── BmsBiosParser.h        # BMS-BIOS 프로토콜 파서
├── tools/
│   ├── bmsbios_bridge.py      # Falcon BMS 공유메모리 → Teensy 시리얼 브릿지
│   ├── dcsbios_bridge.py      # DCS-BIOS UDP → Teensy 시리얼 브릿지
│   └── dcsbios_test_recv.py   # DCS-BIOS UDP 수신 테스트
├── docs/
│   ├── joystick.txt           # 조이스틱 버튼/LED 레이아웃
│   ├── teensy_direct_pins.txt # Teensy 핀 배치표
│   ├── protocol_reference.txt # DCS-BIOS / BMS-BIOS 프로토콜 상세
│   ├── mcp23017_wiring.html   # MCP23017 배선도
│   └── resistor_ladder_wiring.html  # 저항 래더 배선도
└── backup/                    # 이전 버전 아카이브
```

## License

This project is licensed under the [MIT License](LICENSE).

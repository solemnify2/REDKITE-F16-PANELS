# REDKITE F16 PANELS

![My Cockpit](My%20Cockpit.png)

Teensy 4.1 기반 F-16 콕핏 패널 컨트롤러.
**Falcon BMS**(BMS-BIOS)와 **DCS World**(DCS-BIOS) 듀얼 심 지원, 프로토콜 자동 감지.

## Design Goals

VR 전용 미니멀 데스크핏을 지향합니다. VR 헤드셋을 쓰고 손을 뻗은 곳에 스위치가 있었으면 하는 생각을 1년 이상 참다가 결국 만들었습니다 ㅠㅠ 실기 1:1 스케일로 제작했습니다. 사용하지 않을 때 쉽게 치울 수 있는 이동식/수납식 구조이며, 인게임 디스플레이로 충분한 계기류(DED, RWR, 엔진 게이지, HSI 등)는 별도 하드웨어를 제작하지 않고 스위치와 LED 위주로 구성했습니다.

## Features

- **Left Aux Panels 위주로 제작** — I2C(MCP23017)를 이용하여 새로운 패널을 여러 개 추가 가능
- **이동식/수납식 구조** — Left/Right Enclosure를 독립적으로 미니멀하게 제작하여 안 쓸 때 치울 수 있음. PC 데스크톱 본체 위에 커스텀 슬라이딩 마운트를 제작하여 안 쓸 때 테이블 아래로 밀어 넣을 수 있음. ICP 스탠드를 제작하여 안 쓸 때 MFD 쪽으로 내려 모니터 시야를 확보
- **MCP23017 I2C 확장** — MISC 패널은 MCP23017으로 제작하여 Left Aux Panels의 slave 장치로 동작
- **USB Suspend 감지** — PC 절전 시 LED 소등 및 MCU 저전력 모드 진입
- **프로토콜 자동 감지** — BMS-BIOS(바이너리) / DCS-BIOS(바이너리) 자동 판별

## Supported Panels and Input

| Panel | Type | Description |
|-------|------|-------------|
| Gear | Switch, LED | Landing Gear, Hook, Anti-skid, Landing Light, Gear Warning 등 |
| Alt Gear | Switch | ALT Gear Handle / Reset |
| MISC | Switch, LED (MCP23017) | Master ARM, Laser ARM, RF, Autopilot Pitch/Roll, ECM LED 등 |
| CMDS | Switch, Analog Ladder | RWR, JMR, MWS, JETT, MODE (6-pos), PRGM (5-pos) 등 |
| TWA | Analog Ladder, LED | Threat Warning Aux — Search, Act/Pwr, Low, Power |
| Pedal | Analog | Rudder + Left/Right Brake (auto calibration) |

## Hardware

### Panel Sources

패널은 아래 콕핏 빌더 전문 업체에서 구매할 수 있습니다.

- [PC Flights](https://pcflights.com/flight-simulator-cockpit-panels/f-16c-viper-home-cockpit-panels/) — F-16C 패널 전문, 미국/해외 배송

| MISC | Gear | CMDS |
|:----:|:----:|:----:|
| [![MISC](https://pcflights.com/image/cache/catalog/panels/f-16c_viper_misc_panel-600x400.jpg)](https://pcflights.com/f-16c-viper-miscellaneous-misc-panel-flight-sim-part/) | [![Gear](https://pcflights.com/image/cache/catalog/panels/f-16c_viper_landing_gear_panel-600x400.jpg)](https://pcflights.com/f-16c-viper-landing-gear-panel-flight-simulator-module/) | [![CMDS](https://pcflights.com/image/cache/catalog/panels/f-16c_viper_cmds_control_panel-600x400.jpg)](https://pcflights.com/f-16c-viper-cmds-control-panel-module/) |

- [HispaPanel](https://hispapanels.com/tienda/en/20-f-16-fighting-falcon) — 레이저 커팅/각인 패널 키트, 커스텀 제작 가능
- [TEK Creations](https://tekcreations.space/shop/) — LED 백라이트 패널, DCS-BIOS 레디, [Etsy](https://www.etsy.com/shop/TekCreations)에서도 구매 가능

### Components

주요 부품 (MISC, Gear, CMDS, TWA, HMCS, Alt Gear 패널):

| 부품 | 수량 | 비고 |
|------|------|------|
| Teensy 4.1 | 1 | MCU |
| MCP23017 I2C I/O Expander | 1 | MISC 패널용, 선택 |
| 저항 1kΩ / 4.7kΩ (각 100개 세트) | 1 / 1 | 래더 직렬, 풀다운, I2C 풀업 |
| [GX076-30MB](https://www.alibaba.com/product-detail/7-6-Inch-Square-LCD-Display_1601257654342.html) 7.6" Square LCD | 2 | MFD용, Alibaba |
| 토글 스위치 (On-Off) | 13 | |
| 토글 스위치 (On-Off-On) | 6 | |
| 푸시 스위치 (7mm) | 6 | |
| 코리 스위치 | 1 | [HispaPanel](https://hispapanels.com) |
| 택타일 스위치 (6x6x4.5) | 4 | |
| 리밋 스위치 (12.8x6.5) | 3 | |
| 로터리 SR26 1P12T | 2 | |
| 포텐셔미터 (10k) | 1 | |
| 3mm LED (Green / Yellow) | 5 / 1 | |
| 5mm LED (Red / Green) | 1 / 4 | |
| 3mm 고휘도 LED Green (백라이트) | ~100 | 필요수량만큼 |
| SMD 저항 (220Ω / 2.2kΩ / 10kΩ) | 4 / 4 / 1 | |

전체 부품 목록은 [docs/components.xlsx](docs/components.xlsx) 참조.

### 3D Printed Parts

`3d stl/` 폴더에 콕핏 패널 인클로저 및 스탠드의 3D 프린트용 STL 파일이 포함되어 있습니다.
Stand는 각각 Left/Right Enclosure 뒷면에 부착하는 지지대입니다. 에폭시 접착제로 접합을 권장합니다.

| Left Stand | Left Stand (new) | Right Stand | Right Stand (new) |
|:----:|:----:|:----:|:----:|
| <a href="3d%20stl/Left%20Stand.stl"><img src="3d%20stl/Left%20Stand.png" width="150"></a> | <a href="3d%20stl/Left%20Stand%20-%20new.stl"><img src="3d%20stl/Left%20Stand%20-%20new.png" width="150"></a> | <a href="3d%20stl/Right%20Stand.stl"><img src="3d%20stl/Right%20Stand.png" width="150"></a> | <a href="3d%20stl/Right%20Stand-%20new.stl"><img src="3d%20stl/Right%20Stand-%20new.png" width="150"></a> |

| Left Enclosure | Right Enclosure (new) | Enclosure OnePiece |
|:----:|:----:|:----:|
| <img src="3d%20stl/Left%20Enclosure.png" width="150"> | <img src="3d%20stl/Right%20Enclosure%20-%20new.png" width="150"> | <a href="3d%20stl/Enclosure%20-%20%20OnePiece2.stl"><img src="3d%20stl/Enclosure%20-%20%20OnePiece2.png" width="150"></a> |

> **Note:** Left Enclosure, Right Enclosure의 STL 파일은 Cults3D에서 구매한 모델을 수정하여 제작한 것이므로 저작권 보호를 위해 재배포하지 않습니다. 미리보기 이미지만 참고용으로 포함되어 있습니다.

#### 3D Part Sources

패널 완제품을 구매하는 대신 3D 프린터로 직접 출력할 수 있습니다. 패널뿐 아니라 구조물(HUD frame 등)도 구할 수 있습니다. 저는 [Cults3D](https://cults3d.com)에서 구했습니다. 아래 참고하세요.

- [Greenisland](https://cults3d.com/en/users/Greenisland/3d-models) — F-16 콕핏 거의 전체 파트 판매
- [Legarsdusofa](https://cults3d.com/en/design-collections/Legarsdusofa/f-16-cockpit-simulator) — F-16 콕핏 거의 전체 파트 판매
- [The_Viper_Project](https://cults3d.com/en/users/The_Viper_Project/3d-models) — 버튼 노브, 스위치 캡, Dsub 캡 등 무료 제공

## Build

### Arduino IDE Setup

1. [Teensyduino](https://www.pjrc.com/teensy/td_download.html) 설치
2. Arduino IDE에서:
   - **Board**: Teensy 4.1
   - **USB Type**: Serial + Keyboard + Mouse + Joystick (복합 디바이스, Windows 자동 인식)
3. 필요 라이브러리:
   - `Wire` (built-in)
   - `Keyboard` (built-in)

### 스위치류와 Teensy 핀 연결

상세 핀 배치는 [docs/teensy_direct_pins.txt](docs/teensy_direct_pins.txt) 참조.

### Joystick 설정

기본 Joystick은 32버튼까지만 지원합니다. 현재 50버튼을 사용하며, I2C로 패널을 추가하면 더 늘어나므로 128버튼 모드가 필요합니다. Teensy 빌드 전 `%LOCALAPPDATA%\Arduino15\packages\teensy\hardware\avr\<version>\cores\teensy4\usb_desc.h`에서 해당 USB Type의 `JOYSTICK_SIZE`를 `12`에서 `64`로 변경:

```c
#define JOYSTICK_SIZE         64    //  12 = normal, 64 = extreme joystick
```

조이스틱 버튼은 런타임에 순차 번호가 자동 부여되지만, 인게임 내의 키 매핑은 직접 해주셔야 합니다. 페달은 러더 + 독립 브레이크를 합성하며 자동 캘리브레이션을 지원합니다. 현재 배치는 [docs/joystick.txt](docs/joystick.txt) 참조.

### Upload

Arduino IDE에서 `REDKITE_F16_PANELS.ino`를 열고 Upload.

## Customization

스위치/LED 등 하드웨어 변경 시 `REDKITE_F16_PANELS.ino`의 **HARDWARE CONFIGURATION** 섹션의 config 배열만 수정하면 됩니다. 패널을 비활성화하려면 해당 항목을 주석 처리하면 됩니다.

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

## DCS-BIOS / BMS-BIOS Protocol Support

스위치/버튼 입력은 USB Joystick으로 직접 전달되지만, LED 상태는 시뮬레이터에서 받아와야 합니다. Python 브릿지가 시뮬레이터의 내부 데이터를 읽어 시리얼로 Teensy에 전송하고, Teensy는 수신된 프로토콜(BMS-BIOS / DCS-BIOS)을 자동 감지하여 LED를 제어합니다. 시뮬레이터 전환 시 6초 타임아웃 후 자동 재감지.

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

> **참고 — Why not F4ToSerial?** 이전 버전에서는 [F4ToSerial](https://github.com/Music-Junky/F4ToSerial)을 사용했으나, F4TS는 Teensy의 다이렉트 핀만 지정 가능하고 MCP23017 등 I2C 확장 핀을 지원하지 않는 문제가 있어 BMS-BIOS로 대체했습니다. 부수적으로 JSON 파싱 오버헤드 제거, ArduinoJson 의존성 제거 등 소소한 이점도 기대할 수 있습니다 (체감 차이는 미지수). DCS-BIOS/BMS-BIOS 프로토콜을 직접 구현했으므로 향후 DCS나 BMS 업데이트로 주소/오프셋이 변경되면 직접 수정이 필요하지만, 소스가 명확하고(`docs/protocol_reference.txt` 참조) 수정 범위도 작아 큰 부담은 아닙니다. 게다가 [Claude Code](https://claude.com/claude-code)도 있으니 든든합니다.

### DCS World (DCS-BIOS)

- 바이너리 프로토콜, `0x55` sync 4바이트로 자동 감지
- DCS-BIOS 설치 및 UDP 브릿지(`tools/dcsbios_bridge.py`) 필요
- 사용법:
  ```
  pip install pyserial
  python tools/dcsbios_bridge.py COM4
  ```

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
│   └── dcsbios_bridge.py      # DCS-BIOS UDP → Teensy 시리얼 브릿지
├── 3d stl/                    # 3D 프린트용 STL 파일
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

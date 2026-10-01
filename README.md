# ESP32-S3 CYD Hermes Client

ESP32-S3 CYD (Cheap Yellow Display) Hermes integration client.
LVGL dashboard UI, local KWS wakeword, ES8311 audio, WebSocket bridge to Hermes.

## Hardware Profile

- **MCU:** ESP32-S3 (2.4 GHz WiFi/BLE)
- **Display:** ST77922 IPS, 320×480 (landscape), QSPI
- **Audio:** ES8311 I2S codec (mic + speaker)
- **Touch:** I2C capacitive touch (0x55)
- **PSRAM:** 8 MB OPI PSRAM (KWS model + LVGL)
- **Flash:** 16 MB (firmware + LittleFS assets)
- **SD/MMC:** 1-bit mode (GPIO 4/5/6)

## Features

| Feature | Status |
|---------|--------|
| KWS wakeword (local) | ✅ ESP32_KWS, ~500 KB PSRAM |
| ES8311 audio in/out | ✅ I2S 16-bit/16 kHz |
| LVGL dashboard UI | ✅ Status, transcript, image area |
| WebSocket Hermes bridge | ✅ Bidirectional |
| TTS audio playback | ✅ Via ES8311 |
| Image display | ✅ LVGL image object |
| Touch input | ✅ I2C touch controller |
| Local time + weather (idle) | ✅ Status bar |
| SD card fallback | ✅ Model/data storage |

## Architecture

```
┌─────────────────────┐       WebSocket        ┌─────────────────────┐
│   ESP32-S3 CYD      │ ◄────────────────────► │  Hermes Dashboard   │
│                     │                          │  (web/CLI)        │
│ - KWS wakeword      │                          │                     │
│ - LVGL UI           │                          │  Image/data push  │
│ - Audio capture     │                          │  TTS audio push   │
│ - ES8311 I2S        │                          │  Commands receive │
└─────────────────────┘                          └─────────────────────┘
```

No intermediate backend server required. The CYD connects directly to Hermes
via WebSocket.

## Setup

1. Install PlatformIO (VS Code extension or CLI)
2. Connect CYD via USB
3. Run `pull-and-flash-com6.bat` from this repo root
4. Check serial monitor at 115200 baud

## KWS Model

The wakeword model is stored in PSRAM (~500 KB). Supported words are
configurable. Default: "Jarvis" wake word.

## Image Display

Images are received via WebSocket from Hermes dashboard. Supports:
- JPEG (preferred, small footprint)
- PNG (lossless, larger)
- LVGL-compatible image format (pre-decoded)

## Data Widgets

Real-time data can be pushed via WebSocket:
- Sensor readings (temperature, humidity, battery)
- System status (CPU temp, RAM usage)
- Custom text data

## Configuration

Edit `src/config.h` to adjust:
- WiFi credentials (WiFiManager auto-configures on first boot)
- WebSocket server URL
- Wake word list
- Screen timeout
- Image display size

## License

MIT

# ESP32-S3 CYD Hermes — Jarvis wake-word hardware test

**Current status:** ESP-IDF 5.5.3 firmware for the Elecrow 3.5-inch ESP32-S3
(ST77922, ES8311, 16 MB flash, 8 MB octal PSRAM). The new `main/` program
captures 16-kHz audio from the ES8311's I²S microphone and runs Espressif's
bundled `wn9_jarvis_tts` WakeNet9 model locally. It logs `JARVIS DETECTED` on
serial and plays a short tone through the ES8311 speaker. **The display, touch,
Wi-Fi, speech transcription, Hermes networking and TTS playback are not yet
implemented in this firmware.** The old Arduino `src/` files were a nonbuilding
scaffold and are not part of the ESP-IDF build.

This is a microphone/wake-word hardware test, **not yet a Hermes voice client**.
Detection and microphone input still require validation on Mike's board. The
model is TTS-trained; expect to evaluate false triggers and missed detections.
No voice recording is stored or transmitted by this build.

## Build and flash

Install Espressif **ESP-IDF v5.5.3** for the ESP32-S3 and use its exported shell.
The ESP-IDF component manager downloads the pinned `esp-sr` 2.5.5 dependency
and resolves its transitive dependencies. The full build downloads substantial
models/toolchains; allow space and network time on the first build.

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

On Windows, launch an **ESP-IDF 5.5.3 PowerShell/Command Prompt**, then from
this repository run `idf.py set-target esp32s3`, `idf.py build`, and
`idf.py -p COM6 flash monitor` (adjust COM6 to the actual serial port).
Always use **`flash`**, not `app-flash`: ESP-SR builds a separate model partition.
`pull-and-flash-com6.bat` fast-forwards `main`, builds in an ESP-IDF 5.5.3
shell, and flashes **all** partitions to COM6. It requires Git for Windows and
the exported ESP-IDF environment; if either is unavailable, it stops with an
error rather than trying PlatformIO.

The serial monitor should report an ES8311 I²C probe at address `0x18`, the
Jarvis model name and audio chunk size, then a short speaker tone. Speak
"Jarvis"; a detection logs `JARVIS DETECTED` and plays the tone again. If the
microphone slot energies are both zero, check the microphone/codec and serial
logs before judging WakeNet. Amplifier-enable GPIO1 is active low; a speaker
must be attached to the board's speaker connector to hear the tone.

## Implementation notes

- Board wiring is from the archived manufacturer Example_17_echo, with ES8311
  I²S MCLK/BCLK/WS/DOUT/DIN on GPIO17/18/21/15/16, I²C SDA/SCL on GPIO38/39,
  and amp-enable on GPIO1. GPIO16 is **digital I²S input**, not an ADC mic.
- Input is 16-bit 16-kHz stereo bus data. The program logs and selects the
  stronger slot for WakeNet's mono input; this selection needs on-board review.
- The model is compiled into a `model` partition by ESP-SR. Do not put an
  arbitrary `kws_model.bin` on SD; that format was invented by the old scaffold.
- ESP-SR is version-pinned in `main/idf_component.yml`; `sdkconfig.defaults`
  selects only the Jarvis model, octal PSRAM and 16-MB flash. `dependencies.lock`
  is committed after the first successful resolution.
- The `src/` directory is retained only as historical reference and is **not
  built**. It should not be used as an Arduino or PlatformIO project.

## Next integration steps

1. Validate live microphone levels, wake detection and speaker on the actual
   board. This has not been confirmed by a build alone.
2. Port the ST77922 panel/touch using the manufacturer board contract and a
   licensed ESP-IDF display driver; preserve the shared I²C bus ownership.
3. Define an authenticated host-side speech-to-text/audio protocol. WakeNet
   identifies a wake word; it **does not** produce a transcript. Hermes's chat
   API accepts text, not raw PCM, so do not send `Jarvis` as user speech.

## Sources and licensing

- [ESP-SR WakeNet](https://github.com/espressif/esp-sr), version 2.5.5
- [Espressif WakeNet example](https://github.com/espressif/esp-skainet/tree/master/examples/wake_word_detection/wakenet) (CC0/public-domain example)
- Elecrow archived board example: `../esp32-s3-3.5in-st77922-board-reference/source/arduino-demos/Example_17_echo/echo/`

No vendor code is copied into this repository. This repository's original
license declaration is MIT; ESP-SR has its own bundled license. Review upstream
license terms when redistributing firmware and model binaries.

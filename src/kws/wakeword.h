// KWS (Keyword Spotting) wakeword detection on ESP32-S3
// Uses ESP32_KWS library with MFCC + LSTM model stored in PSRAM
// Model file: kws_model.bin (~500KB)

#include <Arduino.h>
#include "ESP32_KWS.h"
#include "include/cyd_pins.h"

namespace {
ESP32_KWS kws;
String lastResult = "Waiting...";
bool audioSilent = false;
uint32_t silenceStart = 0;
constexpr uint32_t SILENCE_THRESHOLD_MS = 800;
}

bool kwsInit() {
    // Load model from SD card or PSRAM
    // Priority: SD > SPIFFS > embedded in code
    if (SD_MMC.begin("/sdcard", true, false)) {
        if (SD_MMC.exists("/kws_model.bin")) {
            kws.loadModel(SD_MMC, "/kws_model.bin");
            Serial.println("KWS: model loaded from SD");
        } else {
            kws.loadModelFromCode();
            Serial.println("KWS: loaded from embedded code");
        }
    } else {
        kws.loadModelFromCode();
        Serial.println("KWS: loaded from embedded code (no SD)");
    }

    kws.start();
    Serial.println("KWS: started");
    return true;
}

bool kwsCheck() {
    // Non-blocking check - returns true if wake word detected
    if (kws.isDetected()) {
        lastResult = kws.getDetectedWord();
        Serial.printf("KWS: detected '%s'\n", lastResult.c_str());
        return true;
    }
    return false;
}

void kwsStartCapture() {
    capturing = true;
    kws.startRecording();
}

void kwsStopCapture() {
    capturing = false;
    kws.stopRecording();
}

bool kwsAudioSilent() {
    // Check last audio buffer for silence
    // Simplified: check if recent samples are near zero
    static uint32_t lastCheck = 0;
    if (millis() - lastCheck < 100) return audioSilent;
    lastCheck = millis();

    int16_t testBuf[64];
    cydMicRead(testBuf, 64);
    uint16_t maxVal = 0;
    for (int i = 0; i < 64; i++) {
        uint16_t absVal = abs(testBuf[i]);
        if (absVal > maxVal) maxVal = absVal;
    }
    audioSilent = (maxVal < 100);  // threshold
    return audioSilent;
}

String kwsGetResult() { return lastResult; }

// ESP32_KWS wakeword module implementation

#include "ESP32_KWS.h"
#include <Arduino.h>
#include "config.h"
#include "audio/capture.h"

namespace {
ESP32_KWS kws;
String lastResult = "Waiting...";
bool audioSilent = false;
uint32_t silenceStart = 0;
uint8_t consecutiveDetections = 0;
}

bool kwsInit() {
    // Load model from SD card or PSRAM
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
    if (kws.isDetected()) {
        lastResult = kws.getDetectedWord();
        Serial.printf("KWS: detected '%s'\n", lastResult.c_str());
        return true;
    }
    return false;
}

void kwsStartCapture() {
    kws.startRecording();
}

void kwsStopCapture() {
    kws.stopRecording();
}

bool kwsAudioSilent() {
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
    audioSilent = (maxVal < MIC_THRESHOLD);
    return audioSilent;
}

String kwsGetResult() { return lastResult; }

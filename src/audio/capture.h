// Audio capture via ES8311 microphone input
// Uses ADC1 for mic input (GPIO16 on CYD)

#include <Arduino.h>
#include "include/cyd_pins.h"

// ─── Configuration ──────────────────────────────────────────────────────
constexpr uint8_t MIC_PIN = 16;
constexpr uint16_t ADC_RESOLUTION = 4095;  // 12-bit
constexpr uint16_t SAMPLES_PER_READ = 256;

// ─── Initialization ─────────────────────────────────────────────────────
bool cydMicInit() {
    pinMode(MIC_PIN, INPUT);
    analogReadResolution(12);
    Serial.println("CYD: Mic initialized");
    return true;
}

// ─── Read Samples ───────────────────────────────────────────────────────
void cydMicRead(int16_t* buf, uint16_t count) {
    for (uint16_t i = 0; i < count; i++) {
        uint16_t raw = analogRead(MIC_PIN);
        // Convert to signed 16-bit
        buf[i] = (int16_t)(raw - 2048);
    }
}

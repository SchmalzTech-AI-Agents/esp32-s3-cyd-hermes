// ESP32-S3 CYD Hermes Client
// Main sketch: LVGL dashboard, KWS wakeword, ES8311 audio, WebSocket bridge
//
// Features:
// - KWS wakeword detection (local, ~500KB on PSRAM)
// - ES8311 I2S audio capture/playback
// - WebSocket connection to Hermes dashboard
// - LVGL UI: status bar, transcript area, data widgets, image display
// - Touch input for navigation
// - Idle: local time + weather display
// - SD card fallback for KWS model if flash < 8MB

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <lvgl.h>
#include <Wire.h>
#include <SD_MMC.h>
#include "include/cyd_pins.h"
#include "src/dashboard/dashboard.h"
#include "src/audio/capture.h"
#include "src/audio/playback.h"
#include "src/kws/wakeword.h"
#include "src/websocket/bridge.h"
#include "src/display/image.h"

// ─── Configuration ───────────────────────────────────────────────────────
constexpr uint16_t WIFI_RECONNECT_MS = 30000;
constexpr uint16_t WSS_HEARTBEAT_MS = 10000;
constexpr uint16_t IDLE_SCREEN_TIMEOUT = 60000;  // 1 min to idle screen
constexpr uint16_t WSS_RECONNECT_MS = 5000;
constexpr uint8_t KWS_SAMPLE_RATE = 16000;
constexpr uint8_t AUDIO_BUF_LEN = 256;  // samples per read

// ─── Global State ───────────────────────────────────────────────────────
volatile bool wakewordTriggered = false;
volatile bool capturing = false;
char lastTranscript[256] = "Waiting...";
bool idleScreenShown = false;

// ─── LVGL Callbacks ─────────────────────────────────────────────────────
static lv_disp_draw_dscr_cb_t disp_draw_dscr_cb;
static lv_img_dsc_t* currentImage = nullptr;

// ─── Setup ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32-S3 CYD Hermes Client ===");

    // Touch controller
    if (!cydTouchInit()) Serial.println("WARN: Touch init failed");

    // Display
    if (!cydDisplayInit()) Serial.println("WARN: Display init failed");

    // Audio
    if (!cydMicInit()) Serial.println("WARN: Mic init failed");
    if (!cydSpeakerInit()) Serial.println("WARN: Speaker init failed");

    // KWS
    if (!kwsInit()) Serial.println("WARN: KWS init failed");

    // LVGL dashboard
    dashboardInit();

    // WiFi
    wifiConnect();

    // WebSocket bridge
    bridgeInit();

    Serial.println("Ready.\n");
}

// ─── Loop ───────────────────────────────────────────────────────────────
void loop() {
    // Check wake word
    if (!capturing && kwsCheck()) {
        Serial.println("KWS: wake word detected");
        dashboardShowListening();
        kwsStartCapture();
    }

    // Read mic samples
    if (capturing) {
        int16_t buf[AUDIO_BUF_LEN];
        cydMicRead(buf, AUDIO_BUF_LEN);
        kwsPush(buf, AUDIO_BUF_LEN);

        // Check for audio completion (silence detection)
        if (kwsAudioSilent()) {
            kwsStopCapture();
            bridgeSendTranscript(kwsGetResult());
            dashboardUpdateTranscript(kwsGetResult());
            dashboardShowStatus("Ready");
        }
    }

    // LVGL render
    lv_task_handler();
    delay(5);

    // Update idle screen if shown
    if (idleScreenShown) {
        dashboardUpdateIdleScreen();
    }

    // Check for idle timeout
    static unsigned long lastTouch = millis();
    if (dashboardHasTouch()) {
        lastTouch = millis();
        idleScreenShown = false;
    } else if (millis() - lastTouch > IDLE_SCREEN_TIMEOUT) {
        dashboardShowIdleScreen();
        idleScreenShown = true;
    }
}

// ─── Helpers ────────────────────────────────────────────────────────────
void wifiConnect() {
    WiFiManager wm;
    wm.setSaveCredentialsCallback([](const char* user, const char* pass) {
        Serial.printf("Credentials: %s / %s\n", user, pass);
    });
    if (!wm.autoConnect("CYD-Hermes")) {
        Serial.println("Failed to connect, restarting");
        ESP.restart();
    }
    Serial.printf("Connected: %s\n", WiFi.localIP().toString().c_str());
}

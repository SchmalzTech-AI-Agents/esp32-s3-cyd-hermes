// Main sketch entry point

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <lvgl.h>
#include <Wire.h>
#include <SD_MMC.h>
#include "config.h"
#include "dashboard/dashboard.h"
#include "audio/capture.h"
#include "audio/playback.h"
#include "kws/wakeword.h"
#include "websocket/bridge.h"
#include "display/image.h"

// ─── Global State ───────────────────────────────────────────────────────
volatile bool wakewordTriggered = false;
volatile bool capturing = false;
char lastTranscript[256] = "Waiting...";
bool idleScreenShown = false;

// ─── WiFi Connect ───────────────────────────────────────────────────────
static void wifiConnect() {
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

// ─── Setup ──────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== ESP32-S3 CYD Hermes Client ===");

    // Touch
    Wire.begin(CYD_PINS_TOUCH_SDA, CYD_PINS_TOUCH_SCL);
    Serial.println("CYD: I2C (touch) initialized");

    // Display
    lv_init();
    lv_port_display_init();
    Serial.println("CYD: display initialized");

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

        // Check for audio completion
        if (kwsAudioSilent()) {
            kwsStopCapture();
            bridgeSendTranscript(kwsGetResult().c_str());
            dashboardUpdateTranscript(kwsGetResult().c_str());
            dashboardShowStatus(STATUS_READY);
        }
    }

    // LVGL render
    lv_task_handler();
    delay(5);

    // Update idle screen if shown
    if (idleScreenShown) {
        dashboardUpdateTime("12:00 PM");
        dashboardUpdateDataWidget("Data: --");
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

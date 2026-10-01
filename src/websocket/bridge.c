// WebSocket bridge - connects ESP32 to Hermes dashboard
// Sends transcript updates, status; receives TTS audio, images, commands

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include <ArduinoJson.h>
#include "config.h"
#include "dashboard/dashboard.h"
#include "audio/playback.h"

namespace {
AsyncWebSocket ws("/ws");
bool wsConnected = false;
}

// ─── Init ───────────────────────────────────────────────────────────────
void bridgeInit() {
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                  AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("WS: connected [%d]\n", client->id());
            wsConnected = true;
            dashboardShowStatus(STATUS_READY);
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.printf("WS: disconnected [%d]\n", client->id());
            wsConnected = false;
            dashboardShowStatus(STATUS_IDLE);
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode == WS_TEXT) {
                // Parse JSON command
                StaticJsonDocument<512> doc;
                DeserializationError err = deserializeJson(doc, (char*)data, len);
                if (err) {
                    Serial.printf("WS: JSON parse failed: %s\n", err.c_str());
                    return;
                }
                const char* type = doc["type"];
                if (strcmp(type, "tts") == 0) {
                    // TTS audio data received
                    Serial.println("WS: TTS audio received");
                } else if (strcmp(type, "image") == 0) {
                    // Image data received (binary)
                    Serial.println("WS: Image received");
                } else if (strcmp(type, "data") == 0) {
                    dashboardUpdateDataWidget(doc["text"] | "No data");
                }
            } else if (info->opcode == WS_BINARY) {
                // Image or audio binary data
                Serial.printf("WS: Binary %u bytes\n", len);
            }
        }
    });

    ws.begin();
    Serial.println("WS: bridge initialized");
}

// ─── Send Functions ─────────────────────────────────────────────────────
void bridgeSendTranscript(const char* text) {
    if (!wsConnected) return;
    StaticJsonDocument<256> doc;
    doc["type"] = "transcript";
    doc["text"] = text;
    char buf[256];
    serializeJson(doc, buf, sizeof(buf));
    ws.textAll(buf);
}

void bridgeSendStatus(const char* status) {
    if (!wsConnected) return;
    StaticJsonDocument<128> doc;
    doc["type"] = "status";
    doc["text"] = status;
    char buf[128];
    serializeJson(doc, buf, sizeof(buf));
    ws.textAll(buf);
}

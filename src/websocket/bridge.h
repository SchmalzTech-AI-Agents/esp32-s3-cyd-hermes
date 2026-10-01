// WebSocket bridge to Hermes dashboard
// Connects ESP32 to Hermes web dashboard via WebSocket
// Sends: transcript updates, status, image requests
// Receives: TTS audio data, image data, commands

#include <Arduino.h>
#include <AsyncWebSocket.h>
#include "include/cyd_pins.h"

namespace {
AsyncWebSocket ws("/ws");
constexpr uint16_t MAX_WS_MSG = 4096;
}

void bridgeInit() {
    ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client,
                  AwsEventType type, void* arg, uint8_t* data, size_t len) {
        if (type == WS_EVT_CONNECT) {
            Serial.printf("WS: client connected [%d]\n", client->id());
        } else if (type == WS_EVT_DATA) {
            AwsFrameInfo* info = (AwsFrameInfo*)arg;
            if (info->opcode == WS_TEXT) {
                // Process text message from Hermes
                String msg((char*)data);
                Serial.printf("WS: received: %s\n", msg.c_str());
                // Parse and act on message
                bridgeProcessMessage(msg);
            }
        } else if (type == WS_EVT_CLOSE) {
            Serial.printf("WS: client disconnected [%d]\n", client->id());
        }
    });

    ws.begin();
    Serial.println("WS: bridge initialized");
}

void bridgeSendTranscript(const char* text) {
    char buf[256];
    snprintf(buf, sizeof(buf), "{\"type\":\"transcript\",\"text\":\"%s\"}", text);
    ws.textAll(buf);
}

void bridgeSendImage(lv_img_dsc_t* img) {
    // Send image binary data over WebSocket
    ws.binaryAll((uint8_t*)img->data, img->data_size);
}

void bridgeProcessMessage(const String& msg) {
    // Parse JSON command from Hermes
    // Expected format: {"type":"tts","text":"Hello world"}
    // Or: {"type":"image","url":"..."}
}

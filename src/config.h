// Configuration for ESP32-S3 CYD Hermes Client

#pragma once

// ─── WiFi ───────────────────────────────────────────────────────────────
constexpr uint16_t WIFI_RECONNECT_MS = 30000;
constexpr uint16_t WIFI_RETRIES = 5;

// ─── WebSocket ──────────────────────────────────────────────────────────
constexpr uint16_t WSS_HEARTBEAT_MS = 10000;
constexpr uint16_t WSS_RECONNECT_MS = 5000;
constexpr uint16_t MAX_WS_MSG = 4096;
constexpr uint16_t MAX_WS_BINARY = 2048;

// ─── Audio ──────────────────────────────────────────────────────────────
constexpr uint8_t AUDIO_BUF_LEN = 256;
constexpr uint16_t MIC_THRESHOLD = 100;  // samples below this = silence
constexpr uint32_t SILENCE_MS = 800;

// ─── Screen ─────────────────────────────────────────────────────────────
constexpr uint16_t IDLE_SCREEN_TIMEOUT = 60000;  // 1 min to idle screen
constexpr uint16_t STATUS_BAR_H = 28;
constexpr uint16_t CONTROLS_H = 40;
constexpr uint16_t SCREEN_W = 480;
constexpr uint16_t SCREEN_H = 320;

// ─── KWS ───────────────────────────────────────────────────────────────
constexpr uint8_t KWS_SAMPLE_RATE = 16000;
constexpr uint8_t KWS_MODEL_SIZE = 512;  // KB in PSRAM
constexpr uint8_t KWS_VAD_THRESHOLD = 3;  // detections before trigger

// ─── Display ────────────────────────────────────────────────────────────
constexpr uint8_t IMAGE_MAX_W = 120;
constexpr uint8_t IMAGE_MAX_H = 120;
constexpr uint8_t TRANSRIPT_MAX_LINES = 8;
constexpr uint8_t TRANSRIPT_LINE_W = 70;

// ─── Status Messages ────────────────────────────────────────────────────
constexpr char* STATUS_READY = "Ready";
constexpr char* STATUS_LISTENING = "Listening";
constexpr char* STATUS_PROCESSING = "Processing";
constexpr char* STATUS_ERROR = "Error";
constexpr char* STATUS_IDLE = "Idle";

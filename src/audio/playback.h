// Audio playback via ES8311 I2S output
// Uses I2S1 peripheral with ES8311 codec at I2C 0x18

#include <Arduino.h>
#include <Wire.h>
#include <driver/i2s_std.h>
#include "include/cyd_pins.h"

namespace {
constexpr uint8_t ES8311_ADDRESS = 0x18;
constexpr uint32_t SAMPLE_RATE = 16000;
constexpr gpio_num_t I2S_MCLK = GPIO_NUM_17;
constexpr gpio_num_t I2S_BCLK = GPIO_NUM_18;
constexpr gpio_num_t I2S_WS = GPIO_NUM_21;
constexpr gpio_num_t I2S_DOUT = GPIO_NUM_15;

i2s_chan_handle_t txChannel = nullptr;
}

bool cydSpeakerInit() {
    pinMode(CydPins::AUDIO_AMP_ENABLE, OUTPUT);
    digitalWrite(CydPins::AUDIO_AMP_ENABLE, LOW);

    // ES8311 codec init
    Wire.begin();
    const struct { uint8_t reg; uint8_t val; } setup[] = {
        {0x00, 0x1F}, {0x00, 0x00}, {0x00, 0x80},
        {0x01, 0x3F}, {0x02, 0x48}, {0x03, 0x10},
        {0x04, 0x10}, {0x05, 0x00}, {0x06, 0x03},
        {0x07, 0x00}, {0x08, 0xFF}, {0x09, 0x0C},
        {0x0A, 0x0C}, {0x0D, 0x01}, {0x0E, 0x02},
        {0x12, 0x00}, {0x13, 0x10}, {0x1C, 0x6A},
        {0x31, 0x00}, {0x32, 0x70}, {0x37, 0x08}
    };
    for (auto& e : setup) {
        Wire.beginTransmission(ES8311_ADDRESS);
        Wire.write(e.reg);
        Wire.write(e.val);
        Wire.endTransmission();
    }

    // I2S init
    i2s_chan_config_t chCfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    i2s_new_channel(&chCfg, &txChannel, nullptr);

    i2s_std_config_t cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCLK, .bclk = I2S_BCLK,
            .ws = I2S_WS, .dout = I2S_DOUT,
            .din = I2S_GPIO_UNUSED
        }
    };
    cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;
    if (i2s_channel_init_std_mode(txChannel, &cfg) != ESP_OK) return false;
    if (i2s_channel_enable(txChannel) != ESP_OK) return false;

    Serial.println("CYD: Speaker initialized");
    return true;
}

void cydSpeakerPlay(const int16_t* buf, uint16_t len) {
    size_t bytesWritten = 0;
    i2s_channel_write(txChannel, buf, len * sizeof(int16_t), &bytesWritten, 1000);
}

void cydSpeakerPlayTone(uint16_t freq, uint32_t durationMs) {
    uint16_t samples = SAMPLE_RATE * durationMs / 1000;
    int16_t buf[256];
    uint32_t phase = 0;
    uint16_t phaseStep = (uint32_t)freq * 32768 / SAMPLE_RATE;

    for (uint32_t i = 0; i < samples; i += 256) {
        uint16_t count = min<uint16_t>(256, samples - i);
        for (uint16_t j = 0; j < count; j++) {
            uint16_t val = (phase & 0x8000) ? (0xFFFF - phase) : phase;
            int16_t sample = (int16_t)((uint32_t)val * 2 - 32768);
            buf[j * 2] = sample;
            buf[j * 2 + 1] = sample;
            phase += phaseStep;
        }
        size_t written = 0;
        i2s_channel_write(txChannel, buf, count * sizeof(int16_t) * 2, &written, 1000);
    }
}

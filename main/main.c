// ESP32-S3 3.5-inch CYD: ES8311 microphone -> WakeNet9 Jarvis.
// This firmware detects only a wake word. It does not transcribe speech or
// connect to a Hermes server; the original Arduino scaffold was not functional.
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_log.h"
#include "esp_wn_iface.h"
#include "esp_wn_models.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "model_path.h"

#define SAMPLE_RATE 16000
#define CODEC_ADDR 0x18
#define I2C_SDA GPIO_NUM_38
#define I2C_SCL GPIO_NUM_39
#define AMP_ENABLE GPIO_NUM_1
#define I2S_MCLK GPIO_NUM_17
#define I2S_BCLK GPIO_NUM_18
#define I2S_WS GPIO_NUM_21
#define I2S_DOUT GPIO_NUM_15
#define I2S_DIN GPIO_NUM_16

static const char *TAG = "cyd_jarvis";
static i2c_master_dev_handle_t codec;
static i2s_chan_handle_t rx;
static i2s_chan_handle_t tx;

static esp_err_t codec_write(uint8_t reg, uint8_t value)
{
    uint8_t payload[2] = {reg, value};
    return i2c_master_transmit(codec, payload, sizeof(payload), 1000);
}

static esp_err_t codec_init(void)
{
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_SDA,
        .scl_io_num = I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus;
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_cfg, &bus), TAG, "I2C bus init failed");
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CODEC_ADDR,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus, &dev_cfg, &codec), TAG, "ES8311 bus registration failed");
    ESP_RETURN_ON_ERROR(i2c_master_probe(bus, CODEC_ADDR, 1000), TAG, "ES8311 not detected at 0x18");

    // 16 kHz, 16-bit stereo, MCLK = 384 * Fs (6.144 MHz). Register values
    // follow the manufacturer's Example_17_echo and ES8311 codec setup.
    static const struct { uint8_t reg, value; } setup[] = {
        {0x00, 0x1f}, {0x00, 0x00}, {0x00, 0x80},
        {0x01, 0x3f}, {0x02, 0x48}, {0x03, 0x10}, {0x04, 0x10},
        {0x05, 0x00}, {0x06, 0x03}, {0x07, 0x00}, {0x08, 0xff},
        {0x09, 0x0c}, {0x0a, 0x0c}, {0x0d, 0x01}, {0x0e, 0x02},
        {0x12, 0x00}, {0x13, 0x10}, {0x1c, 0x6a}, {0x37, 0x08},
        {0x32, 0x19}, // low initial speaker volume
        {0x17, 0xc8}, {0x14, 0x1a}, // analog microphone, ADC gain
    };
    for (size_t i = 0; i < sizeof(setup) / sizeof(setup[0]); ++i) {
        esp_err_t err = codec_write(setup[i].reg, setup[i].value);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "ES8311 write 0x%02x failed: %s", setup[i].reg, esp_err_to_name(err));
            return err;
        }
        if (i == 0) vTaskDelay(pdMS_TO_TICKS(20));
    }
    ESP_LOGI(TAG, "ES8311 configured (I2S mic + speaker)");
    return ESP_OK;
}

static esp_err_t audio_init(void)
{
    gpio_config_t amp_cfg = {
        .pin_bit_mask = 1ULL << AMP_ENABLE,
        .mode = GPIO_MODE_OUTPUT,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&amp_cfg), TAG, "amplifier GPIO failed");
    // Manufacturer AP_ENABLE is active LOW. Keep speaker muted until a beep.
    gpio_set_level(AMP_ENABLE, 1);
    i2s_chan_config_t channel_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    channel_cfg.auto_clear = true;
    ESP_RETURN_ON_ERROR(i2s_new_channel(&channel_cfg, &tx, &rx), TAG, "I2S channels failed");
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_MCLK,
            .bclk = I2S_BCLK,
            .ws = I2S_WS,
            .dout = I2S_DOUT,
            .din = I2S_DIN,
        },
    };
    std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(tx, &std_cfg), TAG, "I2S TX config failed");
    ESP_RETURN_ON_ERROR(i2s_channel_init_std_mode(rx, &std_cfg), TAG, "I2S RX config failed");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(tx), TAG, "I2S TX enable failed");
    ESP_RETURN_ON_ERROR(i2s_channel_enable(rx), TAG, "I2S RX enable failed");
    return codec_init();
}

static void beep(void)
{
    // Short quiet 800 Hz square tone; stereo frames, never outside the buffer.
    int16_t frames[160 * 2];
    for (unsigned i = 0; i < 160; ++i) {
        int16_t sample = (i % 20 < 10) ? 2500 : -2500;
        frames[2 * i] = sample;
        frames[2 * i + 1] = sample;
    }
    gpio_set_level(AMP_ENABLE, 0);
    for (unsigned n = 0; n < 15; ++n) {
        size_t written = 0;
        esp_err_t err = i2s_channel_write(tx, frames, sizeof(frames), &written, 1000);
        if (err != ESP_OK || written != sizeof(frames)) {
            ESP_LOGE(TAG, "Speaker write failed: %s (%u bytes)", esp_err_to_name(err), (unsigned)written);
            break;
        }
    }
    vTaskDelay(pdMS_TO_TICKS(30)); // let DMA drain before muting the amplifier
    gpio_set_level(AMP_ENABLE, 1);
}

void app_main(void)
{
    ESP_LOGI(TAG, "CYD Jarvis wake-word test (no speech transcription / server connection)");
    if (audio_init() != ESP_OK) {
        ESP_LOGE(TAG, "Audio unavailable; not starting WakeNet");
        return;
    }
    srmodel_list_t *models = esp_srmodel_init("model");
    if (!models) {
        ESP_LOGE(TAG, "Model partition missing; flash with 'idf.py flash', not app-flash");
        return;
    }
    char *model_name = esp_srmodel_filter(models, ESP_WN_PREFIX, "jarvis");
    if (!model_name || !strstr(model_name, "jarvis")) {
        ESP_LOGE(TAG, "Jarvis WakeNet model not found in model partition");
        return;
    }
    esp_wn_iface_t *wn = (esp_wn_iface_t *)esp_wn_handle_from_name(model_name);
    if (!wn) {
        ESP_LOGE(TAG, "WakeNet interface unavailable for %s", model_name);
        return;
    }
    model_iface_data_t *model = wn->create(model_name, DET_MODE_95);
    if (!model) {
        ESP_LOGE(TAG, "WakeNet model creation failed");
        return;
    }
    int chunk = wn->get_samp_chunksize(model);
    if (chunk <= 0 || chunk > 4096) {
        ESP_LOGE(TAG, "Invalid WakeNet chunk size: %d", chunk);
        wn->destroy(model);
        return;
    }
    int16_t *stereo = malloc((size_t)chunk * 2 * sizeof(*stereo));
    int16_t *mono = malloc((size_t)chunk * sizeof(*mono));
    if (!stereo || !mono) {
        ESP_LOGE(TAG, "Insufficient audio buffer memory");
        free(stereo); free(mono);
        wn->destroy(model);
        return;
    }
    ESP_LOGI(TAG, "Jarvis model: %s; chunk=%d frames at 16 kHz", model_name, chunk);
    beep();
    int channel = -1; // select codec's populated I2S slot from actual audio
    uint32_t blocks = 0;
    uint64_t window_energy[2] = {0, 0};
    TickType_t cooldown_until = 0;
    for (;;) {
        const size_t target = (size_t)chunk * 2 * sizeof(int16_t);
        size_t received = 0;
        while (received < target) {
            size_t read_now = 0;
            esp_err_t err = i2s_channel_read(rx, (uint8_t *)stereo + received,
                                              target - received, &read_now, 1000);
            if (err != ESP_OK || read_now == 0) {
                ESP_LOGE(TAG, "Microphone read failed: %s", esp_err_to_name(err));
                received = 0;
                break;
            }
            received += read_now;
        }
        if (received != target) continue;
        uint64_t energy[2] = {0, 0};
        for (int i = 0; i < chunk; ++i) {
            int32_t a = stereo[2 * i], b = stereo[2 * i + 1];
            energy[0] += (uint64_t)(a * a);
            energy[1] += (uint64_t)(b * b);
        }
        // The ES8311 is mono on a stereo bus. Compare ten-block energy windows
        // instead of locking to one possibly silent block at startup. A quiet
        // window leaves the slot undecided; a later strong alternate slot can
        // correct an earlier choice without switching on small noise changes.
        window_energy[0] += energy[0];
        window_energy[1] += energy[1];
        if (++blocks % 10 == 0) {
            int candidate = window_energy[1] > window_energy[0] ? 1 : 0;
            uint64_t min_signal = (uint64_t)chunk * 10 * 64; // RMS > 8
            if (window_energy[candidate] > min_signal &&
                (channel < 0 || (candidate != channel &&
                 window_energy[candidate] > window_energy[channel] * 8))) {
                channel = candidate;
                ESP_LOGI(TAG, "Mic slot=%d energy L=%" PRIu64 " R=%" PRIu64,
                         channel, window_energy[0], window_energy[1]);
            } else if (channel < 0 && blocks % 100 == 0) {
                ESP_LOGW(TAG, "No mic signal: energy L=%" PRIu64 " R=%" PRIu64,
                         window_energy[0], window_energy[1]);
            }
            window_energy[0] = window_energy[1] = 0;
        }
        if (channel < 0) continue;
        for (int i = 0; i < chunk; ++i) mono[i] = stereo[2 * i + channel];
        if (xTaskGetTickCount() < cooldown_until) continue;
        if (wn->detect(model, mono) == WAKENET_DETECTED) {
            ESP_LOGI(TAG, "JARVIS DETECTED");
            beep();
            cooldown_until = xTaskGetTickCount() + pdMS_TO_TICKS(2000);
        }
    }
}

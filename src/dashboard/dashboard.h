// Dashboard UI - LVGL layout for CYD 320x480 (landscape)
// Layout: status bar (top), transcript area (center), controls (bottom)

#include <Arduino.h>
#include <lvgl.h>

// ─── LVGL Objects ───────────────────────────────────────────────────────
static lv_obj_t* statusLabel;
static lv_obj_t* transcriptLabel;
static lv_obj_t* statusDot;
static lv_obj_t* timeLabel;
static lv_obj_t* weatherLabel;
static lv_obj_t* imageArea;
static lv_obj_t* dataWidget;

// ─── Constants ──────────────────────────────────────────────────────────
constexpr uint16_t SCREEN_W = 480;
constexpr uint16_t SCREEN_H = 320;
constexpr uint16_t STATUS_BAR_H = 28;
constexpr uint16_t CONTROLS_H = 40;

// ─── Initialization ─────────────────────────────────────────────────────
void dashboardInit() {
    // Status bar background
    lv_obj_t* bar = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, SCREEN_W, STATUS_BAR_H);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_radius(bar, 0, 0);

    statusLabel = lv_label_create(bar);
    lv_label_set_text(statusLabel, "Hermes Client");
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00ff88), 0);
    lv_obj_align(statusLabel, LV_ALIGN_LEFT_MID, 8, 0);

    timeLabel = lv_label_create(bar);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x888888), 0);
    lv_obj_align(timeLabel, LV_ALIGN_RIGHT_MID, -8, 0);

    // Transcript area
    transcriptLabel = lv_label_create(lv_scr_act());
    lv_obj_set_pos(transcriptLabel, 10, STATUS_BAR_H + 5);
    lv_obj_set_size(transcriptLabel, SCREEN_W - 20, SCREEN_H - STATUS_BAR_H - CONTROLS_H - 10);
    lv_obj_set_style_bg_color(transcriptLabel, lv_color_hex(0x2d2d44), 0);
    lv_obj_set_style_radius(transcriptLabel, 8, 0);
    lv_label_set_long_mode(transcriptLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(transcriptLabel, lv_color_hex(0xeeeeee), 0);
    lv_obj_set_style_text_align(transcriptLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_text(transcriptLabel, "Ready. Say wake word to begin.");

    // Status indicator dot
    statusDot = lv_arc_create(lv_scr_act());
    lv_obj_set_pos(statusDot, 4, STATUS_BAR_H + 10);
    lv_obj_set_size(statusDot, 20, 20);
    lv_obj_set_style_arc_color(statusDot, lv_color_hex(0x00ff88), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(statusDot, lv_color_hex(0x333333), 0);
    lv_arc_set_value(statusDot, 100);

    // Image display area
    imageArea = lv_img_create(lv_scr_act());
    lv_obj_set_pos(imageArea, SCREEN_W / 2 - 60, STATUS_BAR_H + 80);
    lv_obj_set_size(imageArea, 120, 120);
    lv_obj_set_style_bg_color(imageArea, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_border_width(imageArea, 1, 0);
    lv_obj_set_style_border_color(imageArea, lv_color_hex(0x444466), 0);

    // Data widget
    dataWidget = lv_label_create(lv_scr_act());
    lv_obj_set_pos(dataWidget, 10, SCREEN_H - CONTROLS_H - 30);
    lv_obj_set_style_text_color(dataWidget, lv_color_hex(0x44ff88), 0);
    lv_label_set_text(dataWidget, "Data: --");

    // Controls bar
    lv_obj_t* ctrl = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(ctrl, 0, SCREEN_H - CONTROLS_H);
    lv_obj_set_size(ctrl, SCREEN_W, CONTROLS_H);
    lv_obj_set_style_bg_color(ctrl, lv_color_hex(0x1a1a2e), 0);

    lv_obj_t* btn = lv_btn_create(ctrl);
    lv_obj_set_pos(btn, 10, 5);
    lv_obj_set_size(btn, 80, 30);
    lv_obj_t* btnLabel = lv_label_create(btn);
    lv_label_set_text(btnLabel, "Stop");
    lv_obj_set_style_bg_color(btn, lv_color_hex(0x880000), 0);
}

// ─── Update Functions ───────────────────────────────────────────────────
void dashboardUpdateTranscript(const char* text) {
    lv_label_set_text(transcriptLabel, text);
}

void dashboardShowStatus(const char* status) {
    lv_label_set_text(statusLabel, status);
    lv_obj_set_style_arc_color(statusDot,
        lv_color_hex(status == "Ready" ? 0x00ff88 : 0xff8800),
        LV_PART_INDICATOR);
}

void dashboardShowListening() {
    lv_label_set_text(transcriptLabel, "Listening...");
    lv_obj_set_style_arc_color(statusDot, lv_color_hex(0xff4444), LV_PART_INDICATOR);
}

void dashboardShowIdleScreen() {
    lv_label_set_text(statusLabel, "Idle");
    lv_obj_set_style_bg_color(transcriptLabel, lv_color_hex(0x1a1a2e), 0);
    lv_label_set_text(transcriptLabel, "Touch to wake");
}

void dashboardUpdateTime(const char* timeStr) {
    lv_label_set_text(timeLabel, timeStr);
}

void dashboardUpdateWeather(const char* weatherStr) {
    if (weatherLabel) lv_label_set_text(weatherLabel, weatherStr);
}

void dashboardUpdateDataWidget(const char* dataStr) {
    lv_label_set_text(dataWidget, dataStr);
}

void dashboardShowImage(lv_img_dsc_t* img) {
    lv_img_set_src(imageArea, img);
}

bool dashboardHasTouch() {
    static unsigned long last = 0;
    // Would check touch coordinates; simplified here
    return false;
}

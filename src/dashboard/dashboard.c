// Dashboard UI implementation - LVGL widgets for status, transcript, image

#include <Arduino.h>
#include <lvgl.h>
#include "config.h"
#include "dashboard.h"

namespace {
lv_obj_t* statusLabel;
lv_obj_t* transcriptLabel;
lv_obj_t* statusDot;
lv_obj_t* timeLabel;
lv_obj_t* imageArea;
lv_obj_t* dataWidget;
lv_obj_t* idleBg;
lv_obj_t* idleLabel;
}

// ─── Initialization ─────────────────────────────────────────────────────
void dashboardInit() {
    // Status bar (top strip)
    lv_obj_t* bar = lv_obj_create(lv_scr_act());
    lv_obj_set_pos(bar, 0, 0);
    lv_obj_set_size(bar, SCREEN_W, STATUS_BAR_H);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_radius(bar, 0, 0);

    statusLabel = lv_label_create(bar);
    lv_label_set_text(statusLabel, STATUS_READY);
    lv_obj_set_style_text_color(statusLabel, lv_color_hex(0x00ff88), 0);
    lv_obj_set_style_text_font(statusLabel, lv_font_montserrat_12, 0);
    lv_obj_align(statusLabel, LV_ALIGN_LEFT_MID, 8, 0);

    timeLabel = lv_label_create(bar);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x888888), 0);
    lv_obj_set_style_text_font(timeLabel, lv_font_montserrat_10, 0);
    lv_obj_align(timeLabel, LV_ALIGN_RIGHT_MID, -8, 0);

    // Transcript area (center)
    transcriptLabel = lv_label_create(lv_scr_act());
    lv_obj_set_pos(transcriptLabel, 10, STATUS_BAR_H + 5);
    lv_obj_set_size(transcriptLabel, SCREEN_W - 20, SCREEN_H - STATUS_BAR_H - CONTROLS_H - 10);
    lv_obj_set_style_bg_color(transcriptLabel, lv_color_hex(0x2d2d44), 0);
    lv_obj_set_style_radius(transcriptLabel, 8, 0);
    lv_label_set_long_mode(transcriptLabel, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(transcriptLabel, lv_color_hex(0xeeeeee), 0);
    lv_obj_set_style_text_align(transcriptLabel, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_style_text_font(transcriptLabel, lv_font_montserrat_11, 0);
    lv_label_set_text(transcriptLabel, "Ready. Say wake word to begin.");

    // Status indicator dot
    statusDot = lv_arc_create(lv_scr_act());
    lv_obj_set_pos(statusDot, 4, STATUS_BAR_H + 10);
    lv_obj_set_size(statusDot, 18, 18);
    lv_obj_set_style_arc_color(statusDot, lv_color_hex(0x00ff88), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(statusDot, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(statusDot, 0, 0);
    lv_arc_set_value(statusDot, 100);

    // Image display area
    imageArea = lv_img_create(lv_scr_act());
    lv_obj_set_pos(imageArea, SCREEN_W / 2 - 60, STATUS_BAR_H + 60);
    lv_obj_set_size(imageArea, IMAGE_MAX_W, IMAGE_MAX_H);
    lv_obj_set_style_bg_color(imageArea, lv_color_hex(0x1a1a2e), 0);
    lv_obj_set_style_border_width(imageArea, 1, 0);
    lv_obj_set_style_border_color(imageArea, lv_color_hex(0x444466), 0);

    // Data widget (bottom)
    dataWidget = lv_label_create(lv_scr_act());
    lv_obj_set_pos(dataWidget, 10, SCREEN_H - CONTROLS_H - 25);
    lv_obj_set_style_text_color(dataWidget, lv_color_hex(0x44ff88), 0);
    lv_obj_set_style_text_font(dataWidget, lv_font_montserrat_10, 0);
    lv_label_set_text(dataWidget, "Data: --");

    // Controls bar (bottom)
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
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xaa2222), LV_STATE_PRESSED);
}

// ─── Update Functions ───────────────────────────────────────────────────
void dashboardUpdateTranscript(const char* text) {
    lv_label_set_text(transcriptLabel, text);
}

void dashboardShowStatus(const char* status) {
    lv_label_set_text(statusLabel, status);
    uint32_t color = (strcmp(status, STATUS_READY) == 0) ? 0x00ff88 :
                     (strcmp(status, STATUS_LISTENING) == 0) ? 0xff4444 :
                     (strcmp(status, STATUS_PROCESSING) == 0) ? 0xffaa00 : 0x888888;
    lv_obj_set_style_arc_color(statusDot, lv_color_hex(color), LV_PART_INDICATOR);
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

void dashboardUpdateDataWidget(const char* dataStr) {
    lv_label_set_text(dataWidget, dataStr);
}

void dashboardShowImage(lv_img_dsc_t* img) {
    lv_img_set_src(imageArea, img);
}

bool dashboardHasTouch() {
    static unsigned long last = 0;
    // Simplified: always returns false until touch callback wired
    return false;
}

// Image display handler - LVGL image rendering from WS or local storage

#include <Arduino.h>
#include <lvgl.h>
#include "dashboard/dashboard.h"

// ─── Functions ──────────────────────────────────────────────────────────
void dashboardShowImage(lv_img_dsc_t* img) {
    lv_img_set_src(imageArea, img);
    Serial.printf("Image displayed: %dx%d\n", img->header.w, img->header.h);
}

void dashboardClearImage() {
    lv_img_set_src(imageArea, NULL);
}

// ─── Image from LVGL binary format ──────────────────────────────────────
lv_img_dsc_t* dashboardCreateImageFromData(const uint8_t* data, uint16_t w, uint16_t h) {
    // LVGL image descriptor - caller must provide data
    static lv_img_dsc_t img;
    img.header.always_zero = 0;
    img.header.flags = 0;
    img.header.w = w;
    img.header.h = h;
    img.data_size = w * h * 2;  // RGB565
    img.data = data;
    return &img;
}

#pragma once

#include <lvgl.h>
#include <Arduino.h>

void dashboardShowImage(lv_img_dsc_t* img);
void dashboardClearImage();
lv_img_dsc_t* dashboardCreateImageFromData(const uint8_t* data, uint16_t w, uint16_t h);

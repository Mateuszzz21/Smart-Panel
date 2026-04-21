è#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <time.h>
#include <sys/time.h>

#include "mbedtls/md.h"
#include "mbedtls/sha256.h"
#include <FS.h>
#include <LittleFS.h>
// ============================
// WIFI
// ============================
static const char* WIFI_SSID = "****";
static const char* WIFI_PASS = "****";

// ============================
// TUYA CLOUD
// ============================
static const char* TUYA_BASE_URL  = "https://openapi.tuyaeu.com";
static const char* TUYA_CLIENT_ID = "f7fupkjsq83e9kpvuc4s";
static const char* TUYA_SECRET    = "329f23b474254a36a23749020180bf5c";
static const char* TUYA_DEVICE_ID = "bf3ac62ebb1d6a165exr53";

// ============================
// NTP / czas
// ============================
static const char* NTP_SERVER_1 = "pool.ntp.org";
static const char* NTP_SERVER_2 = "time.google.com";
static const char* TZ_INFO      = "CET-1CEST,M3.5.0/2,M10.5.0/3";

// ============================
// ENKODER / PRZYCISKI
// ============================

static const int BTN_UP_PIN   = 32;
static const int BTN_DOWN_PIN = 25;
static const int BTN_OK_PIN   = 26;
static const int BTN_BACK_PIN = 27;
// ============================
// LED
// ============================
static const int  LOW_BAT_LED_PIN = 33;      // ustaw pin jaki chcesz użyć
static const bool LOW_BAT_LED_ACTIVE_HIGH = true;

// ============================
// BMP280 / I2C
// ============================
static const uint8_t PANEL_I2C_SDA_PIN = 21;
static const uint8_t PANEL_I2C_SCL_PIN = 22;
static const uint8_t PANEL_BMP280_ADDR = 0x76;   // jeśli nie ruszy, zmień na 0x77
static const uint32_t PANEL_SENSOR_READ_MS = 300000;   // 5 min
static const float TEMP_HOME_OFFSET_C = 0.0f;
// ============================
// PODSWIETLENIE TFT / ODCZUWALNA JASNOSC
// ============================
// Jezeli BL ekranu NIE jest podlaczone do sterowanego pinu, zostaw PWM wylaczone.
// Wtedy ustawienie "Jasnosc" nadal bedzie dzialalo przez zmiane kontrastu UI.
static const bool TFT_BL_PWM_ENABLED = false;
static const int  TFT_BL_PIN         = -1;     // ustaw np. na pin tranzystora BL, jesli go uzywasz
static const int  TFT_BL_PWM_CH      = 0;
static const int  TFT_BL_PWM_FREQ    = 5000;
static const int  TFT_BL_PWM_BITS    = 8;

static const uint8_t TFT_BL_LEVEL_LOW  = 72;
static const uint8_t TFT_BL_LEVEL_MID  = 150;
static const uint8_t TFT_BL_LEVEL_HIGH = 255;

// ============================
// TIMINGI
// ============================
static const uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
static const uint32_t WIFI_RETRY_MS           = 10000;
static const uint32_t TUYA_RETRY_MS           = 8000;
static const uint32_t TUYA_STATUS_REFRESH_MS  = 60000;           // 1 min
static const uint32_t CLOCK_REFRESH_MS        = 1000;
static const uint32_t COLON_BLINK_MS          = 500;
static const uint32_t INPUT_DEBOUNCE_MS       = 160;
static const uint32_t UI_SELECTION_FLASH_MS = 140;

// ============================
// UDP panel <-> zegarek
// ============================
static const uint16_t PANEL_DISCOVERY_PORT    = 4216;
static const uint16_t PANEL_ROOM_DATA_PORT    = 4215;
static const uint32_t PANEL_DISCOVERY_SEND_MS = 10000;

// ============================
// KOLORY RGB565
// ============================
static const uint16_t COL_BG            = 0x18C3;
static const uint16_t COL_TEXT          = 0xFFFF;
static const uint16_t COL_TEXT_MUTED    = 0xBDF7;
static const uint16_t COL_CARD          = 0x31A6;
static const uint16_t COL_CARD_ACTIVE   = 0x5B5D;
static const uint16_t COL_BORDER        = 0x632C;
static const uint16_t COL_BORDER_ACTIVE = 0xFD20;
static const uint16_t COL_OK            = 0x8660;
static const uint16_t COL_ERROR         = 0xF800;
static const uint16_t COL_ACCENT        = 0xD34A;
static const uint16_t COL_WHITE         = 0xFFFF;
// ============================
// NVS / nazwy pokoi
// ============================
static const uint8_t ROOM_NAME_MAX_LEN = 12;
static const char* NVS_NAMESPACE = "smartpanel";


// ============================
// POGODA INTERNETOWA
// ============================
static const bool WEATHER_ENABLED = true;
static const float WEATHER_LAT = 50.0335f;   // Albigowa
static const float WEATHER_LON = 22.2285f;
static const uint32_t WEATHER_REFRESH_MS = 30UL * 60UL * 1000UL;   // 30 min
static const int WEATHER_FORECAST_PLUS_HOURS = 4;

// ============================
// FIRMWARE
// ============================
static const char* FW_VERSION = "STABLE v2.0";

// ============================
// LOKALNE API APP <-> PANEL
// ============================
static const bool API_ENABLED = true;
static const uint16_t API_PORT = 80;
static const char* API_TOKEN = "123456";
static const bool API_ALLOW_CORS = true;

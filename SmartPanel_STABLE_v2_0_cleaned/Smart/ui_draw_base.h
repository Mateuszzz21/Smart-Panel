#pragma once
#include <FS.h>
#include <LittleFS.h>
#include "objects.h"
#include "state.h"
#include "config.h"
#include "ui_draw_icons.h"
#include "room_config.h"

inline String fmt1(float v) {
  char buf[12];
  dtostrf(v, 0, 1, buf);
  return String(buf);
}

inline bool isRoomLowBat(int room) {
  if (room < 0 || room >= ROOM_COUNT) return false;
  return (g_roomFlags[room] & 0x01) != 0;
}

inline bool isSalonLowBat() {
  return isRoomLowBat(ROOM_SALON);
}

inline bool isGabinetLowBat() {
  return isRoomLowBat(ROOM_GABINET);
}

inline bool isOutLowBat() {
  return (g_outFlags & 0x01) != 0;
}

inline bool isAnySystemLowBat() {
  return isSalonLowBat() || isGabinetLowBat() || isOutLowBat();
}

inline void initLowBatLed() {
  pinMode(LOW_BAT_LED_PIN, OUTPUT);

  if (LOW_BAT_LED_ACTIVE_HIGH) digitalWrite(LOW_BAT_LED_PIN, LOW);
  else                         digitalWrite(LOW_BAT_LED_PIN, HIGH);
}

inline void updateLowBatLed() {
  const bool on = isAnySystemLowBat();

  if (LOW_BAT_LED_ACTIVE_HIGH) digitalWrite(LOW_BAT_LED_PIN, on ? HIGH : LOW);
  else                         digitalWrite(LOW_BAT_LED_PIN, on ? LOW : HIGH);
}

inline uint8_t brightnessToPwmDuty(uint8_t setting) {
  switch (setting) {
    case 0: return 70;
    case 1: return 140;
    default: return 255;
  }
}

inline void applyBacklightLevel() {
  if (!TFT_BL_PWM_ENABLED || TFT_BL_PIN < 0) return;
  ledcWrite(TFT_BL_PIN, brightnessToPwmDuty(g_brightnessSetting));
}

inline bool drawBackgroundFromBin(const char* path) {
  fs::File f = LittleFS.open(path, "r");
  if (!f) {
    Serial.println("[BG] open fail");
    return false;
  }

  static uint16_t lineBuf[480];

  for (int y = 0; y < 320; y++) {
    size_t need = 480 * 2;
    size_t got = f.read((uint8_t*)lineBuf, need);

    if (got != need) {
      f.close();
      Serial.println("[BG] read fail");
      return false;
    }

    tft.pushImage(0, y, 480, 1, lineBuf);
  }

  f.close();
  return true;
}

inline bool drawBackgroundClip(const char* path, int clipX, int clipY, int clipW, int clipH) {
  fs::File f = LittleFS.open(path, "r");
  if (!f) {
    return false;
  }

  static uint16_t rowBuf[480];

  for (int y = clipY; y < clipY + clipH; y++) {
    uint32_t offset = (uint32_t)y * 480UL * 2UL;
    f.seek(offset);

    size_t got = f.read((uint8_t*)rowBuf, 480 * 2);
    if (got != 480 * 2) {
      f.close();
      return false;
    }

    tft.pushImage(clipX, y, clipW, 1, &rowBuf[clipX]);
  }

  f.close();
  return true;
}

inline void initDisplay() {
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(false);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(g_colText, g_colBg);
  tft.setTextDatum(TL_DATUM);
}

inline void drawBootScreen(const char* txt) {
  tft.fillScreen(g_colBg);
  tft.setTextColor(g_colText, g_colBg);
  tft.setTextDatum(MC_DATUM);
  tft.drawString(txt, 240, 160, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawBackgroundPanel() {
  if (!drawBackgroundFromBin("/bg_wave_480x320_rgb565.bin")) {
    tft.fillScreen(TFT_BLACK);
  }
}

inline void drawWiFiTuyaBottom() {
  if (!drawBackgroundClip("/bg_wave_480x320_rgb565.bin", 6, 240, 468, 64)) {
    tft.fillRect(6, 240, 468, 64, TFT_BLACK);
  }

  const int wifiX = 10;
  const int wifiY = 270;

  switch (g_wifiUiState) {
    case WIFI_UI_CONNECTED:
      drawWiFiOkIcon(wifiX, wifiY);
      break;
    case WIFI_UI_CONNECTING:
      drawWiFiConnectingIcon(wifiX, wifiY);
      break;
    default:
      drawWiFiLostIcon(wifiX, wifiY);
      break;
  }

  const int tuyaX = 372;
  const int tuyaY = 244;

  switch (g_tuyaUiState) {
    case TUYA_UI_CONNECTED:
      drawTuyaOkIcon(tuyaX, tuyaY);
      break;
    case TUYA_UI_CONNECTING:
      drawTuyaConnectingIcon(tuyaX, tuyaY);
      break;
    default:
      drawTuyaLostIcon(tuyaX, tuyaY);
      break;
  }

  g_prevWifiOk = g_wifiOk;
  g_prevWifiUiState = g_wifiUiState;
  g_prevTuyaOk = g_tuyaOk;
  g_prevTuyaUiState = g_tuyaUiState;
}

inline bool isUiSelectionFlashActive() {
  return (millis() - g_uiSelectFlashMs) < UI_SELECTION_FLASH_MS;
}

inline void drawBatteryMiniIcon(int x, int y, int w, int h, uint8_t pct, bool lowBat) {
  uint16_t col = lowBat ? g_colError : g_colAccent;
  tft.drawRoundRect(x, y, w, h, 3, col);
  tft.fillRect(x + w, y + h / 4, 3, h / 2, col);
  int innerW = w - 4;
  int fillW = map((int)pct, 0, 100, 0, innerW);
  if (fillW < 0) fillW = 0;
  if (fillW > innerW) fillW = innerW;
  if (fillW > 0) tft.fillRoundRect(x + 2, y + 2, fillW, h - 4, 2, col);
}

inline void drawLowBatBadge(int x, int y, int w, const String& txt) {
  tft.fillRoundRect(x, y, w, 24, 10, g_colError);
  tft.drawRoundRect(x, y, w, 24, 10, g_colWhite);
  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(txt, x + w / 2, y + 5, 2);
  tft.setTextDatum(TL_DATUM);
}
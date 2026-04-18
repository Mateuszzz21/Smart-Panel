#pragma once
#include <FS.h>
#include <LittleFS.h>
#include "objects.h"
#include "state.h"
#include "config.h"
#include "ui_icons_all.h"

static const uint16_t ICON_TRANSPARENT_KEY = 0xF81F;

static const char* ICON_MAIN_HOME = "/home_icon_60x52_rgb565_le.bin";

static const char* ICON_SALON_MENU     = "/salon_sofa_60x50_rgb565_le.bin";
static const char* ICON_SYPIALNIA_MENU = "/sypialnia_icon_60x50_rgb565_le.bin";
static const char* ICON_GABINET_MACHINE = "/machine_icon_60x50_rgb565_le.bin";
static const char* ICON_GABINET_PRINTER = "/printer_icon_60x50_rgb565_le.bin";
static const char* ICON_SETTINGS_GEAR = "/settings_gear_48x48_rgb565_le.bin";
static const char* ICON_BACK_ARROW_TILE = "/back_arrow_tile_40x28_rgb565_le.bin";

static const char* ICON_ROOM_TEMP = "/room_temp_icon_60x52_rgb565_le.bin";
static const char* ICON_ROOM_HUM = "/room_humidity_icon_60x52_v1_rgb565_le.bin";
static const char* ICON_TILE_BULB_OFF = "//tile_bulb_off_48x48_rgb565_le.bin";
static const char* ICON_TILE_BULB_ON  = "/tile_bulb_on_48x48_rgb565_le.bin";

static const char* ICON_DAY_CLEAR   = "/icon_day_clear_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_PARTLY  = "/icon_day_partly_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_CLOUDY  = "/icon_day_cloudy_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_FOG     = "/icon_mgla_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_RAIN    = "/icon_deszcz_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_SNOW    = "/icon_snieg_wzorcowe_50x50_rgb565.bin";
static const char* ICON_DAY_STORM   = "/icon_burza_wzorcowe_50x50_rgb565.bin";

static const char* ICON_NIGHT_CLEAR  = "/icon_night_clear_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_PARTLY = "/icon_night_partly_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_CLOUDY = "/icon_night_cloudy_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_FOG    = "/icon_mgla_noc_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_RAIN   = "/icon_deszcz_noc_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_SNOW   = "/icon_snieg_noc_wzorcowe_50x50_rgb565.bin";
static const char* ICON_NIGHT_STORM  = "/icon_burza_noc_wzorcowe_50x50_rgb565.bin";

inline bool isTransparent(uint16_t c) {
  uint8_t r = ((c >> 11) & 0x1F) << 3;
  uint8_t g = ((c >> 5)  & 0x3F) << 2;
  uint8_t b = (c & 0x1F) << 3;
  return (r > 200 && b > 200 && g < 80);
}

inline void drawTransparentIcon(int x, int y, int w, int h, const uint16_t* bmp) {
  for (int yy = 0; yy < h; yy++) {
    for (int xx = 0; xx < w; xx++) {
      uint16_t c = pgm_read_word(&bmp[yy * w + xx]);
      if (!isTransparent(c)) {
        tft.drawPixel(x + xx, y + yy, c);
      }
    }
  }
}

inline bool isNearTransparentKey565(uint16_t c, uint16_t key = ICON_TRANSPARENT_KEY) {
  int cr = (c >> 11) & 0x1F;
  int cg = (c >> 5)  & 0x3F;
  int cb = c & 0x1F;

  int kr = (key >> 11) & 0x1F;
  int kg = (key >> 5)  & 0x3F;
  int kb = key & 0x1F;

  return (abs(cr - kr) <= 1) && (abs(cg - kg) <= 1) && (abs(cb - kb) <= 1);
}

inline bool drawTransparentBinIcon(const char* path, int x, int y, int w, int h, uint16_t transparentKey = ICON_TRANSPARENT_KEY) {
  fs::File f = LittleFS.open(path, "r");
  if (!f) return false;

  static uint16_t rowBuf[96];
  if (w > 96) {
    f.close();
    return false;
  }

  for (int yy = 0; yy < h; yy++) {
    size_t need = (size_t)w * 2;
    size_t got = f.read((uint8_t*)rowBuf, need);
    if (got != need) {
      f.close();
      return false;
    }

    for (int xx = 0; xx < w; xx++) {
      uint16_t c = rowBuf[xx];
      if (c != transparentKey && !isNearTransparentKey565(c, transparentKey)) {
        tft.drawPixel(x + xx, y + yy, c);
      }
    }
  }

  f.close();
  return true;
}

inline void drawBackArrowIcon(int x, int y, uint16_t color = TFT_WHITE) {
  (void)color;
  const uint16_t colCyan    = 0x07FF;
  const uint16_t colMagenta = 0xF81F;
  const uint16_t colWhite   = TFT_WHITE;

  tft.drawLine(x + 7,  y + 12, x + 17, y + 4,  colMagenta);
  tft.drawLine(x + 7,  y + 12, x + 17, y + 20, colMagenta);
  tft.drawLine(x + 8,  y + 12, x + 18, y + 4,  colMagenta);
  tft.drawLine(x + 8,  y + 12, x + 18, y + 20, colMagenta);
  tft.drawLine(x + 10, y + 12, x + 21, y + 12, colMagenta);
  tft.drawLine(x + 10, y + 13, x + 21, y + 13, colMagenta);

  tft.drawLine(x + 5,  y + 12, x + 15, y + 5,  colCyan);
  tft.drawLine(x + 5,  y + 12, x + 15, y + 19, colCyan);
  tft.drawLine(x + 6,  y + 12, x + 16, y + 5,  colCyan);
  tft.drawLine(x + 6,  y + 12, x + 16, y + 19, colCyan);
  tft.drawLine(x + 8,  y + 12, x + 20, y + 12, colCyan);
  tft.drawLine(x + 8,  y + 11, x + 20, y + 11, colCyan);

  tft.drawLine(x + 6,  y + 12, x + 14, y + 6,  colWhite);
  tft.drawLine(x + 6,  y + 12, x + 14, y + 18, colWhite);
  tft.drawLine(x + 8,  y + 12, x + 18, y + 12, colWhite);
}

inline void drawHomeIconPretty(int x, int y) {
  drawTransparentBinIcon(ICON_MAIN_HOME, x, y, 60, 52);
}
inline void drawBackArrowTileIcon(int x, int y) {
  drawTransparentBinIcon(ICON_BACK_ARROW_TILE, x, y, 40, 28);
}
//inline void drawOutdoorIconPretty(int x, int y)      { drawTransparentIcon(x, y, 45, 40, icon_out_top); }

inline void drawTempIcon(int x, int y, uint16_t) {
  drawTransparentBinIcon(ICON_ROOM_TEMP, x, y, 60, 52);
  }

  inline void drawHumidityIcon(int x, int y, uint16_t) {
  drawTransparentBinIcon(ICON_ROOM_HUM, x, y, 60, 52);
}
inline void drawPowerOnIcon(int x, int y, uint16_t) {
  drawTransparentBinIcon(ICON_TILE_BULB_ON, x, y, 48, 48);
}
inline void drawPowerOffIcon(int x, int y, uint16_t) {
  drawTransparentBinIcon(ICON_TILE_BULB_OFF, x, y, 48, 48);
}

inline void drawWiFiOkIcon(int x, int y)         { drawTransparentIcon(x, y, 40, 40, icon_wifi_ok_40); }
inline void drawWiFiLostIcon(int x, int y)       { drawTransparentIcon(x, y, 40, 40, icon_wifi_lost_40); }
inline void drawWiFiConnectingIcon(int x, int y) { drawTransparentIcon(x, y, 40, 40, icon_wifi_connecting_40); }

inline void drawTuyaOkIcon(int x, int y) {
  drawTransparentBinIcon("/tuya_ok_flat_92x52_rgb565.bin", x, y, 92, 52);
}

inline void drawTuyaLostIcon(int x, int y) {
  drawTransparentBinIcon("/tuya_off_flat_92x52_rgb565.bin", x, y, 92, 52);
}

inline void drawTuyaConnectingIcon(int x, int y) {
  drawTransparentBinIcon("/tuya_ok_flat_92x52_rgb565.bin", x, y, 92, 52);
}
inline void drawSalonMenuIcon(int x, int y) {
  drawTransparentBinIcon(ICON_SALON_MENU, x, y, 60, 50);
}
inline void drawSypialniaMenuIcon(int x, int y) {
  drawTransparentBinIcon(ICON_SYPIALNIA_MENU, x, y, 60, 50);
}
inline void drawGabinetMachineIcon(int x, int y) {
  drawTransparentBinIcon(ICON_GABINET_MACHINE, x, y, 60, 50);
}

inline void drawGabinetPrinterIcon(int x, int y) {
  drawTransparentBinIcon(ICON_GABINET_PRINTER, x, y, 60, 50);
}
inline void drawSettingsGearIcon(int x, int y) {
  drawTransparentBinIcon(ICON_SETTINGS_GEAR, x, y, 48, 48);
}

inline bool isNightWeatherIcons() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return false;
  int hh = timeinfo.tm_hour;
  return (hh >= 20 || hh < 6);
}

inline void drawWeatherCodeIconDay(int x, int y, uint8_t code) {
  switch (code) {
    case 0:
      drawTransparentBinIcon(ICON_DAY_CLEAR, x, y, 50, 50);
      break;

    case 1:
    case 2:
      drawTransparentBinIcon(ICON_DAY_PARTLY, x, y, 50, 50);
      break;

    case 3:
      drawTransparentBinIcon(ICON_DAY_CLOUDY, x, y, 50, 50);
      break;

    case 45:
    case 48:
      drawTransparentBinIcon(ICON_DAY_FOG, x, y, 50, 50);
      break;

    case 51:
    case 53:
    case 55:
    case 61:
    case 63:
    case 65:
    case 80:
    case 81:
    case 82:
      drawTransparentBinIcon(ICON_DAY_RAIN, x, y, 50, 50);
      break;

    case 71:
    case 73:
    case 75:
    case 85:
    case 86:
      drawTransparentBinIcon(ICON_DAY_SNOW, x, y, 50, 50);
      break;

    case 95:
    case 96:
    case 99:
      drawTransparentBinIcon(ICON_DAY_STORM, x, y, 50, 50);
      break;

    default:
      drawTransparentBinIcon(ICON_DAY_CLOUDY, x, y, 50, 50);
      break;
  }
}

inline void drawWeatherCodeIconNight(int x, int y, uint8_t code) {
  switch (code) {
    case 0:
      drawTransparentBinIcon(ICON_NIGHT_CLEAR, x, y, 50, 50);
      break;

    case 1:
    case 2:
      drawTransparentBinIcon(ICON_NIGHT_PARTLY, x, y, 50, 50);
      break;

    case 3:
      drawTransparentBinIcon(ICON_NIGHT_CLOUDY, x, y, 50, 50);
      break;

    case 45:
    case 48:
      drawTransparentBinIcon(ICON_NIGHT_FOG, x, y, 50, 50);
      break;

    case 51:
    case 53:
    case 55:
    case 61:
    case 63:
    case 65:
    case 80:
    case 81:
    case 82:
      drawTransparentBinIcon(ICON_NIGHT_RAIN, x, y, 50, 50);
      break;

    case 71:
    case 73:
    case 75:
    case 85:
    case 86:
      drawTransparentBinIcon(ICON_NIGHT_SNOW, x, y, 50, 50);
      break;

    case 95:
    case 96:
    case 99:
      drawTransparentBinIcon(ICON_NIGHT_STORM, x, y, 50, 50);
      break;

    default:
      drawTransparentBinIcon(ICON_NIGHT_CLOUDY, x, y, 50, 50);
      break;
  }
}

inline void drawWeatherCodeIcon(int x, int y, uint8_t code) {
  if (isNightWeatherIcons()) {
    drawWeatherCodeIconNight(x, y, code);
  } else {
    drawWeatherCodeIconDay(x, y, code);
  }
}
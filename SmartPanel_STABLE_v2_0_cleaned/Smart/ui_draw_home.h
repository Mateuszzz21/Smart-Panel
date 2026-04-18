#pragma once
#include "ui_draw_base.h"
#include "ui_draw_icons.h"

inline void drawBackgroundClipHome(int x, int y, int w, int h) {
  if (!drawBackgroundClip("/bg_wave_480x320_rgb565.bin", x, y, w, h)) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
  }
}

inline void redrawSingleWeatherIconBg(int x, int y) {
  drawBackgroundClipHome(x - 8, y - 8, 66, 66);
}

inline void drawHomeLeftBlock() {
  drawBackgroundClipHome(0, 24, 145, 78);

  drawHomeIconPretty(4, 26);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(fmt1(g_tempHome) + " C", 78, 44, 4);

  g_prevTempHome = g_tempHome;
}

inline void drawHomeRightBlock() {
  drawBackgroundClipHome(320, 10, 160, 124);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(fmt1(g_tempOut) + " C", 378, 34, 4);
  tft.setTextDatum(TL_DATUM);

  redrawSingleWeatherIconBg(422, 8);
  redrawSingleWeatherIconBg(422, 62);

  drawWeatherCodeIcon(422, 8, g_weatherNowCode);
  drawWeatherCodeIcon(422, 62, g_weatherPlus4hCode);

  g_prevTempOut = g_tempOut;
  g_prevWeatherNowCode = g_weatherNowCode;
  g_prevWeatherPlus4hCode = g_weatherPlus4hCode;
}

inline void drawHomeWeatherIconsOnly() {
  redrawSingleWeatherIconBg(422, 8);
  redrawSingleWeatherIconBg(422, 62);

  drawWeatherCodeIcon(422, 8, g_weatherNowCode);
  drawWeatherCodeIcon(422, 62, g_weatherPlus4hCode);

  g_prevWeatherNowCode = g_weatherNowCode;
  g_prevWeatherPlus4hCode = g_weatherPlus4hCode;
}

inline void drawHomeDateBlock() {
  drawBackgroundClipHome(150, 18, 180, 82);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(g_dateStr, 240, 34, 4);

  tft.setTextColor(g_colTextMuted);
  tft.drawCentreString(g_dayStr, 240, 64, 4);

  tft.setTextDatum(TL_DATUM);

  g_prevDateStr = g_dateStr;
  g_prevDayStr  = g_dayStr;
}

inline void drawClockBase() {
  drawBackgroundClipHome(36, 86, 372, 130);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);

  tft.drawCentreString(g_timeHH, 176, 140, 8);
  tft.drawCentreString(g_timeMM, 312, 140, 8);

  tft.setTextDatum(TL_DATUM);

  g_prevTimeHH = g_timeHH;
  g_prevTimeMM = g_timeMM;
}

inline bool isAnyHomeLowBat() {
  return isSalonLowBat() || isGabinetLowBat() || isOutLowBat();
}

inline void drawHomeLowBatArea() {
  drawBackgroundClipHome(110, 222, 260, 94);

  int y = 224;
  const int step = 28;

  if (isSalonLowBat()) {
    drawLowBatBadge(145, y, 190, "SALON LOW BAT");
    y += step;
  }

  if (isGabinetLowBat()) {
    drawLowBatBadge(145, y, 190, "PRACOWNIA LOW BAT");
    y += step;
  }

  if (isOutLowBat()) {
    drawLowBatBadge(145, y, 190, "OUT LOW BAT");
  }

  g_prevHomeAnyLowBat = isAnyHomeLowBat();
  g_prevHomeSalonLowBat = isSalonLowBat();
  g_prevHomeGabinetLowBat = isGabinetLowBat();
  g_prevHomeOutLowBat = isOutLowBat();
}

inline void drawClockColonOnly() {
  drawBackgroundClipHome(228, 156, 32, 52);

  const int cx = 244;
  const int cyTop = 168;
  const int cyBot = 196;
  const int r = 4;

  if (g_colonVisible) {
    tft.fillCircle(cx, cyTop, r, g_colWhite);
    tft.fillCircle(cx, cyBot, r, g_colWhite);
  }

  g_prevColonVisible = g_colonVisible;
}

inline void drawHomeStatic() {
  drawBackgroundPanel();
  drawHomeLeftBlock();
  drawHomeDateBlock();
  drawHomeRightBlock();
  drawClockBase();
  drawClockColonOnly();
  drawWiFiTuyaBottom();
  drawHomeLowBatArea();
  g_forceFullDynamic = false;
}

inline void drawHomeDynamic() {
  if (g_tempHome != g_prevTempHome) {
  drawHomeLeftBlock();
  drawHomeDateBlock();
}
  if (g_tempOut != g_prevTempOut) drawHomeRightBlock();
  if (g_dateStr != g_prevDateStr || g_dayStr != g_prevDayStr) drawHomeDateBlock();

bool bottomChanged = false;

if (g_wifiOk != g_prevWifiOk ||
    g_wifiUiState != g_prevWifiUiState ||
    g_tuyaOk != g_prevTuyaOk ||
    g_tuyaUiState != g_prevTuyaUiState) {
  drawWiFiTuyaBottom();
  bottomChanged = true;
}

  if (g_weatherNowCode != g_prevWeatherNowCode ||
      g_weatherPlus4hCode != g_prevWeatherPlus4hCode) {
    drawHomeWeatherIconsOnly();
  }

  if (g_timeHH != g_prevTimeHH || g_timeMM != g_prevTimeMM) {
    drawClockBase();
    drawClockColonOnly();
  } else if (g_colonVisible != g_prevColonVisible) {
    drawClockColonOnly();
  }

const bool salonChanged   = (isSalonLowBat()   != g_prevHomeSalonLowBat);
const bool gabinetChanged = (isGabinetLowBat() != g_prevHomeGabinetLowBat);
const bool outChanged     = (isOutLowBat()     != g_prevHomeOutLowBat);
const bool anyChanged     = (isAnyHomeLowBat() != g_prevHomeAnyLowBat);

if (bottomChanged || salonChanged || gabinetChanged || outChanged || anyChanged) {
  drawHomeLowBatArea();
}
}
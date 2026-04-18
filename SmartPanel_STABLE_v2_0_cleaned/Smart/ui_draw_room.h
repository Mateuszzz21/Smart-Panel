#pragma once
#include "ui_draw_base.h"
#include "ui_draw_icons.h"
#include "room_logic.h"
#include "room_config.h"

inline void drawBackgroundClipRoom(int x, int y, int w, int h) {
  if (!drawBackgroundClip("/bg_wave_480x320_rgb565.bin", x, y, w, h)) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
  }
}

inline void drawRoomHeader() {
  drawBackgroundClipRoom(20, 20, 440, 40);

  drawBackArrowTileIcon(16, 18);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(g_roomTitle, 240, 28, 4);
  tft.setTextDatum(TL_DATUM);

  const int room = roomIndexFromPage(g_currentPage);
  if (room == ROOM_GABINET || room == ROOM_SALON) {
    const bool lowBat = isRoomLowBat(room);

    drawBatteryMiniIcon(382, 24, 30, 14, g_roomBattPct[room], lowBat);

    tft.setTextColor(lowBat ? g_colError : g_colWhite);
    tft.drawString(String(g_roomBattPct[room]) + "%", 420, 20, 2);
  }
}

inline void drawRoomMetrics() {
  drawBackgroundClipRoom(34, 76, 412, 102);

  drawTempIcon(44, 88, g_colAccent);
  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(fmt1(g_roomTemp) + " C", 116, 100, 4);

  drawHumidityIcon(246, 92, g_colAccent);
  tft.drawString(fmt1(g_roomHum) + " %RH", 316, 100, 4);

  const int room = roomIndexFromPage(g_currentPage);
  if (room == ROOM_GABINET || room == ROOM_SALON) {
    const bool lowBat = isRoomLowBat(room);

    if (lowBat) {
      drawLowBatBadge(166, 154, 148, "LOW BAT");
    }

    g_prevRoomBattMv  = g_roomBattMv[room];
    g_prevRoomBattPct = g_roomBattPct[room];
    g_prevRoomFlags   = g_roomFlags[room];
  }

  g_prevRoomTemp = g_roomTemp;
  g_prevRoomHum  = g_roomHum;
}

inline void drawControlTileIconState(bool isOn, int x, int y, uint16_t fg) {
  if (isOn) drawPowerOnIcon(x, y, fg);
  else      drawPowerOffIcon(x, y, fg);
}

inline void drawControlTile(int idx) {
  Rect r = g_controlRects[idx];
  const bool selected = (g_selectedControlTile == idx);
  const bool flash = selected && isUiSelectionFlashActive();
  const bool isBedroom = (roomIndexFromPage(g_currentPage) == ROOM_SYPIALNIA);
  const bool isOn = isBedroom ? g_tuyaSwitchStates[idx] : false;

  drawBackgroundClipRoom(r.x, r.y, r.w, r.h);

  if (selected) {
    tft.fillRoundRect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, 15,
                      flash ? g_colCardActive : g_colBg);
  }

  tft.drawRoundRect(r.x, r.y, r.w, r.h, 16,
                    selected ? (flash ? g_colBorderActive : g_colAccent) : g_colBorder);

  if (selected) {
    tft.fillRoundRect(r.x + 16, r.y + r.h - 8, r.w - 32, flash ? 6 : 5, 2, g_colAccent);
  }

  drawControlTileIconState(isOn, r.x + 60, r.y + 6, g_colWhite);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(isOn ? "ON" : "OFF", r.x + r.w / 2, r.y + 64, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawRoomStatic() {
  drawBackgroundPanel();
  drawRoomHeader();
  drawRoomMetrics();
  for (int i = 0; i < 2; i++) drawControlTile(i);

  g_prevSelectedControlTile = g_selectedControlTile;
  g_prevTuyaSwitchStates[0] = g_tuyaSwitchStates[0];
  g_prevTuyaSwitchStates[1] = g_tuyaSwitchStates[1];
  g_forceFullDynamic = false;
}

inline void drawRoomDynamic() {
  const int room = roomIndexFromPage(g_currentPage);
  const bool hasBatteryUi = (room == ROOM_GABINET || room == ROOM_SALON);

  bool needHeader = false;
  bool needMetrics = false;

  if (g_roomTemp != g_prevRoomTemp || g_roomHum != g_prevRoomHum || g_roomTitle.length() == 0) {
    needMetrics = true;
    needHeader = true;
  }

  if (hasBatteryUi) {
    if (g_roomBattMv[room] != g_prevRoomBattMv ||
        g_roomBattPct[room] != g_prevRoomBattPct ||
        g_roomFlags[room] != g_prevRoomFlags) {
      needHeader = true;
      needMetrics = true;
    }
  }

  if (needHeader) drawRoomHeader();
  if (needMetrics) drawRoomMetrics();

  const bool statesChanged =
    (g_tuyaSwitchStates[0] != g_prevTuyaSwitchStates[0]) ||
    (g_tuyaSwitchStates[1] != g_prevTuyaSwitchStates[1]);

  if (statesChanged) {
    drawControlTile(0);
    drawControlTile(1);
    g_prevTuyaSwitchStates[0] = g_tuyaSwitchStates[0];
    g_prevTuyaSwitchStates[1] = g_tuyaSwitchStates[1];
  }

  if (g_selectedControlTile != g_prevSelectedControlTile) {
    if (g_prevSelectedControlTile >= 0 && g_prevSelectedControlTile < 2) {
      drawControlTile(g_prevSelectedControlTile);
    }
    drawControlTile(g_selectedControlTile);
    g_prevSelectedControlTile = g_selectedControlTile;
  }
}

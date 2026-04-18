#pragma once
#include "ui_draw_base.h"
#include "ui_draw_icons.h"
#include "ui_icons_all.h"

inline void drawBackgroundClipMenu(int x, int y, int w, int h) {
  if (!drawBackgroundClip("/bg_wave_480x320_rgb565.bin", x, y, w, h)) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
  }
}

inline void drawMenuHeader() {
  drawBackgroundClipMenu(20, 20, 440, 40);
  drawBackArrowTileIcon(16, 18);
  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString("MENU GLOWNE", 240, 28, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawMenuItemIcon(int idx, int x, int y) {
  switch (idx) {
    case 0: drawSalonMenuIcon(x - 30, y - 18); break;

    case 1:
      drawGabinetMachineIcon(x - 66, y - 20);  // maszyna bardziej w lewo
      drawGabinetPrinterIcon(x + 18,  y - 20);  // drukarka bardziej w prawo
      break;

    case 2: drawSypialniaMenuIcon(x - 24, y - 20); break;
    case 3: drawSettingsGearIcon(x - 24, y - 20); break;
  }
}

inline void drawMenuItem(int idx) {
  Rect r = g_menuRects[idx];
  const bool selected = (g_selectedMenuItem == idx);
  const bool flash = selected && isUiSelectionFlashActive();

  drawBackgroundClipMenu(r.x, r.y, r.w, r.h);

  if (selected) {
    tft.fillRoundRect(r.x + 1, r.y + 1, r.w - 2, r.h - 2, 15,
                      flash ? g_colCardActive : g_colBg);
  }

  tft.drawRoundRect(r.x, r.y, r.w, r.h, 16,
                    selected ? (flash ? g_colBorderActive : g_colAccent) : g_colBorder);

  drawMenuItemIcon(idx, r.x + r.w / 2, r.y + 22);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(g_menuItems[idx].label, r.x + r.w / 2, r.y + 54, 4);
  tft.setTextDatum(TL_DATUM);

  if (selected) {
    tft.fillRoundRect(r.x + 16, r.y + r.h - 8, r.w - 32, flash ? 6 : 5, 2, g_colAccent);
  }
}

inline void drawMenuStatic() {
  drawBackgroundPanel();
  drawMenuHeader();
  for (int i = 0; i < 4; i++) drawMenuItem(i);
  g_prevSelectedMenuItem = g_selectedMenuItem;
  g_forceFullDynamic = false;
}

inline void drawMenuDynamic() {
  if (g_selectedMenuItem != g_prevSelectedMenuItem) {
    if (g_prevSelectedMenuItem >= 0 && g_prevSelectedMenuItem < 4) {
      drawMenuItem(g_prevSelectedMenuItem);
    }
    drawMenuItem(g_selectedMenuItem);
    g_prevSelectedMenuItem = g_selectedMenuItem;
  }
}

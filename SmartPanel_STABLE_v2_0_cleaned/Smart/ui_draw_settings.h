#pragma once
#include "ui_draw_base.h"
#include "state.h"
#include "room_logic.h"
#include "ui_draw_icons.h"

void applyTheme();
void saveUserSettings();

inline void drawBackgroundClipSettings(int x, int y, int w, int h) {
  if (!drawBackgroundClip("/bg_wave_480x320_rgb565.bin", x, y, w, h)) {
    tft.fillRect(x, y, w, h, TFT_BLACK);
  }
}

inline const char* brightnessText() {
  switch (g_brightnessSetting) {
    case 0: return "NISKA";
    case 1: return "SREDNIA";
    default: return "WYSOKA";
  }
}

inline const char* themeText() {
  switch (g_themeSetting) {
    case 0: return "WARM";
    case 1: return "BLUE";
    default: return "GRAPHITE";
  }
}

inline const char* autoHomeText() {
  switch (g_autoHomeSetting) {
    case 0: return "15 s";
    case 1: return "30 s";
    case 2: return "60 s";
    default: return "OFF";
  }
}

inline const char* wifiUiText() {
  switch (g_wifiUiState) {
    case WIFI_UI_CONNECTED:  return "OK";
    case WIFI_UI_CONNECTING: return "LACZY";
    default:                 return "BRAK";
  }
}

inline const char* tuyaUiText() {
  switch (g_tuyaUiState) {
    case TUYA_UI_CONNECTED:  return "OK";
    case TUYA_UI_CONNECTING: return "LACZY";
    default:                 return "BRAK";
  }
}

inline String formatUptime() {
  uint32_t s = millis() / 1000UL;
  uint32_t d = s / 86400UL;
  s %= 86400UL;
  uint32_t h = s / 3600UL;
  s %= 3600UL;
  uint32_t m = s / 60UL;

  char buf[32];
  snprintf(buf, sizeof(buf), "%lu d %02lu:%02lu",
           (unsigned long)d,
           (unsigned long)h,
           (unsigned long)m);
  return String(buf);
}

inline String formatUdpAge() {
  if (g_lastUdpPacketMs == 0) return "BRAK";

  uint32_t ageSec = (millis() - g_lastUdpPacketMs) / 1000UL;
  char buf[24];
  snprintf(buf, sizeof(buf), "%lu s temu", (unsigned long)ageSec);
  return String(buf);
}

inline String ipStringOrDash() {
  if (!g_wifiOk) return "-";
  return WiFi.localIP().toString();
}

inline String rssiStringOrDash() {
  if (!g_wifiOk) return "-";
  return String((int)WiFi.RSSI()) + " dBm";
}

inline void drawSettingsHeader(const char* title, const char* backText) {
  (void)backText;

  drawBackgroundClipSettings(20, 20, 440, 40);

  drawBackArrowTileIcon(16, 18);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(title, 240, 28, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawSettingsHubTile(int idx, int x, int y, int w, int h, const char* title, const char* subtitle) {
  (void)subtitle;

  const bool selected = (g_selectedSettingsHubItem == idx);
  const bool flash = selected && isUiSelectionFlashActive();

  drawBackgroundClipSettings(x, y, w, h);

  if (selected) {
    tft.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 15,
                      flash ? g_colCardActive : g_colBg);
  }

  tft.drawRoundRect(x, y, w, h, 16,
                    selected ? (flash ? g_colBorderActive : g_colAccent) : g_colBorder);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(title, x + w / 2, y + 28, 4);
  tft.setTextDatum(TL_DATUM);

  if (selected) {
    tft.fillRoundRect(x + 16, y + h - 8, w - 32, flash ? 6 : 5, 2, g_colAccent);
  }
}

inline void drawSettingsPage() {
  drawBackgroundPanel();
  drawSettingsHeader("USTAWIENIA", "BACK = MENU");

  drawSettingsHubTile(0, 36, 88, 190, 82, "INFORMACJE", "");
  drawSettingsHubTile(1, 254, 88, 190, 82, "EKRAN", "");
  drawSettingsHubTile(2, 36, 186, 190, 82, "NAZWY POKOI", "");
  drawSettingsHubTile(3, 254, 186, 190, 82, "IKONY", "");

  g_prevSelectedSettingsHubItem = g_selectedSettingsHubItem;
  g_forceFullDynamic = false;
}

inline void drawSettingsDynamic() {
  if (g_selectedSettingsHubItem != g_prevSelectedSettingsHubItem) {
    drawSettingsHubTile(0, 36, 88, 190, 82, "INFORMACJE", "");
    drawSettingsHubTile(1, 254, 88, 190, 82, "EKRAN", "");
    drawSettingsHubTile(2, 36, 186, 190, 82, "NAZWY POKOI", "");
    drawSettingsHubTile(3, 254, 186, 190, 82, "IKONY", "");
    g_prevSelectedSettingsHubItem = g_selectedSettingsHubItem;
  }
}

inline void drawScreenSettingRow(int idx, int y, const char* label, const char* value) {
  const bool selected = (g_selectedSettingsItem == idx);
  const bool flash = selected && isUiSelectionFlashActive();

  drawBackgroundClipSettings(42, y, 396, 52);

  if (selected) {
    tft.fillRoundRect(43, y + 1, 394, 50, 15, flash ? g_colCardActive : g_colBg);
  }

  tft.drawRoundRect(42, y, 396, 52, 16,
                    selected ? (flash ? g_colBorderActive : g_colAccent) : g_colBorder);

  tft.setTextColor(g_colTextMuted);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(label, 60, y + 16, 2);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(value, 418, y + 12, 4);

  tft.setTextDatum(TL_DATUM);

  if (selected) {
    tft.fillRoundRect(60, y + 42, 360, flash ? 6 : 5, 2, g_colAccent);
  }
}

inline void drawScreenSettingsPage() {
  drawBackgroundPanel();
  drawSettingsHeader("EKRAN", "BACK = USTAWIENIA");

  drawScreenSettingRow(0, 82,  "Jasnosc",    brightnessText());
  drawScreenSettingRow(1, 152, "Motyw",      themeText());
  drawScreenSettingRow(2, 222, "Auto powrot", autoHomeText());

  g_prevSelectedSettingsItem = g_selectedSettingsItem;
  g_forceFullDynamic = false;
}

inline void drawScreenSettingsDynamic() {
  static uint8_t prevBrightness = 255;
  static uint8_t prevTheme = 255;
  static uint8_t prevAutoHome = 255;

  bool selectionChanged = (g_selectedSettingsItem != g_prevSelectedSettingsItem);
  bool valueChanged =
      (g_brightnessSetting != prevBrightness) ||
      (g_themeSetting != prevTheme) ||
      (g_autoHomeSetting != prevAutoHome);

  if (!selectionChanged && !valueChanged && !g_forceFullDynamic) return;

  drawScreenSettingRow(0, 82,  "Jasnosc",    brightnessText());
  drawScreenSettingRow(1, 152, "Motyw",      themeText());
  drawScreenSettingRow(2, 222, "Auto powrot", autoHomeText());

  g_prevSelectedSettingsItem = g_selectedSettingsItem;
  prevBrightness = g_brightnessSetting;
  prevTheme = g_themeSetting;
  prevAutoHome = g_autoHomeSetting;
  g_forceFullDynamic = false;
}

inline void drawInfoRow4(int y,
                         const char* labelL, const String& valueL,
                         const char* labelR, const String& valueR) {
  drawBackgroundClipSettings(22, y, 436, 34);

  const int label1X = 28;
  const int value1X = 214;
  const int label2X = 246;
  const int value2X = 452;

  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(g_colTextMuted);
  tft.drawString(labelL, label1X, y + 6, 4);
  tft.drawString(labelR, label2X, y + 6, 4);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(valueL, value1X, y + 6, 4);
  tft.drawString(valueR, value2X, y + 6, 4);

  tft.setTextDatum(TL_DATUM);
}

inline void drawInfoRow4Ip(int y,
                           const char* labelL, const String& valueL,
                           const char* labelR, const String& valueR) {
  drawBackgroundClipSettings(22, y, 436, 34);

  const int label1X = 28;
  const int value1X = 214;
  const int label2X = 246;
  const int value2X = 452;

  tft.setTextDatum(TL_DATUM);

  tft.setTextColor(g_colTextMuted);
  tft.drawString(labelL, label1X, y + 6, 4);
  tft.drawString(labelR, label2X, y + 6, 4);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(valueL, value1X, y + 8, 2);
  tft.drawString(valueR, value2X, y + 6, 4);

  tft.setTextDatum(TL_DATUM);
}

inline void drawInfoValuesOnly4(int y,
                                const String& valueL,
                                const String& valueR) {
  drawBackgroundClipSettings(108, y, 112, 34);
  drawBackgroundClipSettings(332, y, 126, 34);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(valueL, 214, y + 6, 4);
  tft.drawString(valueR, 452, y + 6, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawInfoValuesOnly4Ip(int y,
                                  const String& valueL,
                                  const String& valueR) {
  drawBackgroundClipSettings(108, y, 112, 34);
  drawBackgroundClipSettings(332, y, 126, 34);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TR_DATUM);
  tft.drawString(valueL, 214, y + 8, 2);
  tft.drawString(valueR, 452, y + 6, 4);
  tft.setTextDatum(TL_DATUM);
}

inline void drawInfoPage() {
  drawBackgroundPanel();
  drawSettingsHeader("INFORMACJE", "BACK = USTAWIENIA");

  drawInfoRow4Ip(64, "IP", ipStringOrDash(), "RSSI", rssiStringOrDash());
  drawInfoRow4(102, "WiFi", String(wifiUiText()), "Tuya", String(tuyaUiText()));
  drawInfoRow4(140, "Uptime", formatUptime(), "UDP", formatUdpAge());
  drawInfoRow4(178, "HOME", fmt1(g_tempHome) + " C", "OUT", fmt1(g_tempOut) + " C");
  drawInfoRow4(216, "BAT OUT", String(g_outBattPct) + " %", "V OUT", String((float)g_outBattMv / 1000.0f, 2) + " V");
  drawInfoRow4(254, "LOW", g_outLowBat ? "TAK" : "NIE", "FW", FW_VERSION);

  g_forceFullDynamic = false;
}

inline void drawInfoDynamic() {
  static uint32_t lastInfoRefreshMs = 0;
  if (millis() - lastInfoRefreshMs < 1000) return;
  lastInfoRefreshMs = millis();

  drawInfoValuesOnly4Ip(64, ipStringOrDash(), rssiStringOrDash());
  drawInfoValuesOnly4(102, String(wifiUiText()), String(tuyaUiText()));
  drawInfoValuesOnly4(140, formatUptime(), formatUdpAge());
  drawInfoValuesOnly4(178, fmt1(g_tempHome) + " C", fmt1(g_tempOut) + " C");
  drawInfoValuesOnly4(216, String(g_outBattPct) + " %", String((float)g_outBattMv / 1000.0f, 2) + " V");
  drawInfoValuesOnly4(254, g_outLowBat ? "TAK" : "NIE", FW_VERSION);
}

inline void drawRoomNameRow(int idx, int y) {
  const bool selected = (g_selectedRoomNameItem == idx);
  String title = roomDisplayName(idx);

  drawBackgroundClipSettings(54, y, 372, 46);

  if (selected) {
    tft.fillRoundRect(55, y + 1, 370, 44, 13, g_colBg);
  }

  tft.drawRoundRect(54, y, 372, 46, 14, selected ? g_colAccent : g_colBorder);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, 72, y + 10, 4);

  tft.setTextDatum(TR_DATUM);
  tft.drawString("EDYTUJ", 404, y + 22, 2);
  tft.setTextDatum(TL_DATUM);

  if (selected) {
    tft.fillRoundRect(70, y + 38, 340, 4, 2, g_colAccent);
  }
}

inline void drawRoomNamesPage() {
  drawBackgroundPanel();
  drawSettingsHeader("NAZWY POKOI", "BACK = USTAWIENIA");

  drawRoomNameRow(0, 84);
  drawRoomNameRow(1, 144);
  drawRoomNameRow(2, 204);

  g_prevSelectedRoomNameItem = g_selectedRoomNameItem;
  g_forceFullDynamic = false;
}

inline void drawRoomNamesDynamic() {
  if (g_selectedRoomNameItem != g_prevSelectedRoomNameItem) {
    drawRoomNameRow(0, 84);
    drawRoomNameRow(1, 144);
    drawRoomNameRow(2, 204);
    g_prevSelectedRoomNameItem = g_selectedRoomNameItem;
  }
}

inline void drawIconsMenuTile(int idx, int x, int y, int w, int h,
                              const char* line1, const char* line2) {
  const bool selected = (g_selectedIconsMenuItem == idx);
  const bool flash = selected && isUiSelectionFlashActive();

  drawBackgroundClipSettings(x, y, w, h);

  if (selected) {
    tft.fillRoundRect(x + 1, y + 1, w - 2, h - 2, 15,
                      flash ? g_colCardActive : g_colBg);
  }

  tft.drawRoundRect(x, y, w, h, 16,
                    selected ? (flash ? g_colBorderActive : g_colAccent) : g_colBorder);

  if (idx == 0) {
    drawHomeIconPretty(x + (w - 45) / 2, y + 10);
  } else if (idx == 1) {
    drawTransparentBinIcon(ICON_DAY_PARTLY, x + (w - 50) / 2, y + 8, 50, 50);
  } else if (idx == 2) {
    drawTransparentBinIcon(ICON_NIGHT_PARTLY, x + (w - 50) / 2, y + 8, 50, 50);
  }

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(line1, x + w / 2, y + 68, 2);
  tft.drawCentreString(line2, x + w / 2, y + 88, 2);
  tft.setTextDatum(TL_DATUM);

 if (selected) {
  tft.fillRoundRect(x + 18, y + h - 3, w - 36, flash ? 5 : 4, 2, g_colAccent);
}
}

inline void drawIconsPage() {
  drawBackgroundPanel();
  drawSettingsHeader("IKONY", "BACK = USTAWIENIA");
  drawBackgroundClipSettings(18, 62, 444, 236);

  drawIconsMenuTile(0, 26, 112, 130, 104, "IKONY", "GLOWNE");
  drawIconsMenuTile(1, 175, 112, 130, 104, "IKONY", "DZIENNE");
  drawIconsMenuTile(2, 324, 112, 130, 104, "IKONY", "NOCY");

  g_prevSelectedIconsMenuItem = g_selectedIconsMenuItem;
  g_forceFullDynamic = false;
}

inline void drawIconsDynamic() {
  if (g_selectedIconsMenuItem != g_prevSelectedIconsMenuItem) {
    drawIconsMenuTile(0, 26, 112, 130, 104, "IKONY", "GLOWNE");
    drawIconsMenuTile(1, 175, 112, 130, 104, "IKONY", "DZIENNE");
    drawIconsMenuTile(2, 324, 112, 130, 104, "IKONY", "NOCY");
    g_prevSelectedIconsMenuItem = g_selectedIconsMenuItem;
  }
}

inline void drawIconsMainPage() {
  drawBackgroundPanel();
  drawSettingsHeader("IKONY GLOWNE", "BACK = IKONY");
  drawBackgroundClipSettings(18, 62, 444, 236);

  tft.setTextDatum(TL_DATUM);

  // ===== NAGLOWKI SEKCJI =====
  tft.setTextColor(g_colWhite);
  tft.drawString("HOME", 34, 86, 2);
  tft.drawString("ROOM", 180, 86, 2);
  tft.drawString("MENU", 332, 86, 2);

  // ===== HOME =====
  drawHomeIconPretty(34, 108);
  tft.drawString("DOM", 108, 126, 2);

  drawBackArrowTileIcon(34, 176);
  tft.drawString("BACK", 108, 192, 2);

  drawSettingsGearIcon(34, 236);
  tft.drawString("USTAW.", 108, 252, 2);

  // ===== ROOM =====
  drawTempIcon(180, 108, g_colAccent);
  tft.drawString("TEMP", 254, 126, 2);

  drawHumidityIcon(180, 176, g_colAccent);
  tft.drawString("HUM", 254, 194, 2);

  drawPowerOffIcon(168, 220, g_colWhite);
  tft.drawString("OFF", 228, 234, 2);

  drawPowerOnIcon(246, 220, g_colWhite);
  tft.drawString("ON", 316, 234, 2);

  // ===== MENU =====
  drawSalonMenuIcon(332, 108);
  tft.drawString("SALON", 404, 126, 2);

  drawGabinetMachineIcon(312, 164);
  drawGabinetPrinterIcon(378, 164);
  tft.drawString("PRAC.", 336, 206, 2);

  drawSypialniaMenuIcon(332, 232);
  tft.drawString("SYPIALNIA", 398, 246, 2);

  g_forceFullDynamic = false;
}

inline void drawIconPreview(const char* path, int iconX, int iconY, const char* label, int textX, int textY) {
  drawTransparentBinIcon(path, iconX, iconY, 50, 50);
  tft.drawString(label, textX, textY, 2);
}

inline void drawIconsDayPage() {
  drawBackgroundPanel();
  drawSettingsHeader("IKONY DZIENNE", "BACK = IKONY");
  drawBackgroundClipSettings(18, 62, 444, 236);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("DZIEN", 36, 74, 2);

  drawIconPreview(ICON_DAY_CLEAR,  24,  96, "CZYSTE",  20, 146);
  drawIconPreview(ICON_DAY_PARTLY, 130, 96, "CZESC.", 130, 146);
  drawIconPreview(ICON_DAY_CLOUDY, 236, 96, "POCHM.", 232, 146);
  drawIconPreview(ICON_DAY_FOG,    342, 96, "MGLA",   350, 146);

  drawIconPreview(ICON_DAY_RAIN,   77, 196, "DESZCZ",  72, 246);
  drawIconPreview(ICON_DAY_SNOW,  198, 196, "SNIEG",  198, 246);
  drawIconPreview(ICON_DAY_STORM, 319, 196, "BURZA",  321, 246);

  tft.setTextDatum(TL_DATUM);
  g_forceFullDynamic = false;
}

inline void drawIconsNightPage() {
  drawBackgroundPanel();
  drawSettingsHeader("IKONY NOCY", "BACK = IKONY");
  drawBackgroundClipSettings(18, 62, 444, 236);

  tft.setTextColor(g_colWhite);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("NOC", 36, 74, 2);

  drawIconPreview(ICON_NIGHT_CLEAR,  24,  96, "CZYSTE",  20, 146);
  drawIconPreview(ICON_NIGHT_PARTLY, 130, 96, "CZESC.", 130, 146);
  drawIconPreview(ICON_NIGHT_CLOUDY, 236, 96, "POCHM.", 232, 146);
  drawIconPreview(ICON_NIGHT_FOG,    342, 96, "MGLA",   350, 146);

  drawIconPreview(ICON_NIGHT_RAIN,   77, 196, "DESZCZ",  72, 246);
  drawIconPreview(ICON_NIGHT_SNOW,  198, 196, "SNIEG",  198, 246);
  drawIconPreview(ICON_NIGHT_STORM, 319, 196, "BURZA",  321, 246);

  tft.setTextDatum(TL_DATUM);
  g_forceFullDynamic = false;
}

inline void drawRoomNameEditCharBox(int i, const String& current) {
  const int boxW = 28;
  const int gap = 6;
  const int totalW = ROOM_NAME_MAX_LEN * boxW + (ROOM_NAME_MAX_LEN - 1) * gap;
  const int startX = (480 - totalW) / 2;
  const int x = startX + i * (boxW + gap);
  const int boxY = 126;

  const bool selected = (i == g_editCharIndex);

  drawBackgroundClipSettings(x, boxY, boxW, 54);

  if (selected) {
    tft.fillRoundRect(x + 1, boxY + 1, boxW - 2, 52, 8, g_colBg);
  }

  tft.drawRoundRect(x, boxY, boxW, 54, 9, selected ? g_colAccent : g_colBorder);

  char c = (i < current.length()) ? current[i] : ' ';
  tft.setTextColor(g_colWhite);
  tft.setTextDatum(MC_DATUM);
  tft.drawCentreString(String(c), x + boxW / 2, boxY + 28, 4);
  tft.setTextDatum(TL_DATUM);

  if (selected) {
    tft.fillRoundRect(x + 6, boxY + 48, boxW - 12, 4, 2, g_colAccent);
  }
}

inline void drawRoomNameEditPage() {
  drawBackgroundPanel();
  drawSettingsHeader("EDYCJA NAZWY", "BACK = ZAPISZ");

  String title = roomDisplayName(g_editRoomIndex);
  tft.setTextColor(g_colTextMuted);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Pokoj:", 56, 82, 2);

  tft.setTextColor(g_colWhite);
  tft.drawString(title, 150, 78, 4);

  const String current = g_roomNames[g_editRoomIndex];

  for (int i = 0; i < ROOM_NAME_MAX_LEN; i++) {
    drawRoomNameEditCharBox(i, current);
  }

  drawBackgroundClipSettings(56, 228, 320, 60);

  tft.setTextColor(g_colTextMuted);
  tft.drawString("Obrot = znak", 56, 232, 2);
  tft.drawString("Klik = nastepna pozycja", 56, 252, 2);

  char activeChar = (g_editCharIndex < current.length()) ? current[g_editCharIndex] : ' ';
  tft.drawString(String("Aktualny znak: ") + activeChar, 56, 272, 2);

  g_prevEditCharIndex = g_editCharIndex;
  g_prevEditRoomValue = current;
  g_forceFullDynamic = false;
}

inline void drawRoomNameEditDynamic() {
  const String current = g_roomNames[g_editRoomIndex];

  if (g_prevEditRoomValue != current || g_prevEditCharIndex != g_editCharIndex) {
    for (int i = 0; i < ROOM_NAME_MAX_LEN; i++) {
      drawRoomNameEditCharBox(i, current);
    }

    drawBackgroundClipSettings(56, 268, 200, 22);
    char activeChar = (g_editCharIndex < current.length()) ? current[g_editCharIndex] : ' ';
    tft.setTextColor(g_colTextMuted);
    tft.setTextDatum(TL_DATUM);
    tft.drawString(String("Aktualny znak: ") + activeChar, 56, 272, 2);

    g_prevEditRoomValue = current;
    g_prevEditCharIndex = g_editCharIndex;
  }
}
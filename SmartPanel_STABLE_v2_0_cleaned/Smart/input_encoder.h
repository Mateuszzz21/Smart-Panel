#pragma once
#include "config.h"
#include "state.h"
#include "room_logic.h"
#include "tuya_client.h"
#include "pages_home.h"
#include "ui_draw_base.h"

void saveUserSettings();

inline void initEncoderInput() {
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_OK_PIN, INPUT_PULLUP);
  pinMode(BTN_BACK_PIN, INPUT_PULLUP);
}

inline bool isRoomPage(PageId p) {
  return (p == PAGE_ROOM_SALON || p == PAGE_ROOM_GABINET || p == PAGE_ROOM_SYPIALNIA);
}

inline void markUserActivity() {
  g_lastUserActionMs = millis();
}

inline void goToPage(PageId page) {
  g_prevPage = g_currentPage;
  g_currentPage = page;

  if (page == PAGE_HOME) {
    g_prevTimeHH = "";
    g_prevTimeMM = "";
    g_prevDateStr = "";
    g_prevDayStr = "";
    g_prevColonVisible = !g_colonVisible;
    g_prevTempHome = -999.0f;
    g_prevTempOut  = -999.0f;
    g_prevWifiUiState = (WiFiUiState)(-1);
    g_prevTuyaUiState = (TuyaUiState)(-1);
  }

  if (page == PAGE_SETTINGS_SCREEN)     g_prevSelectedSettingsItem = -1;
  if (page == PAGE_SETTINGS)            g_prevSelectedSettingsHubItem = -1;
  if (page == PAGE_SETTINGS_ROOMS)      g_prevSelectedRoomNameItem = -1;
  if (page == PAGE_SETTINGS_ICONS_MENU) g_prevSelectedIconsMenuItem = -1;

  if (isRoomPage(page)) {
    g_prevRoomBattMv  = 0xFFFF;
    g_prevRoomBattPct = 255;
    g_prevRoomFlags   = 255;
  }

  forceFullDynamicRefresh();
}

inline void selectPrevMenuItem() { g_selectedMenuItem = (g_selectedMenuItem + 3) % 4; }
inline void selectNextMenuItem() { g_selectedMenuItem = (g_selectedMenuItem + 1) % 4; }

inline void selectPrevControlTile() { g_selectedControlTile = (g_selectedControlTile + 1) % 2; }
inline void selectNextControlTile() { g_selectedControlTile = (g_selectedControlTile + 1) % 2; }

inline void selectPrevSettingsItem() { g_selectedSettingsItem = (g_selectedSettingsItem + SCREEN_SETTING_COUNT - 1) % SCREEN_SETTING_COUNT; }
inline void selectNextSettingsItem() { g_selectedSettingsItem = (g_selectedSettingsItem + 1) % SCREEN_SETTING_COUNT; }

inline void selectPrevSettingsHubItem() { g_selectedSettingsHubItem = (g_selectedSettingsHubItem + 3) % 4; }
inline void selectNextSettingsHubItem() { g_selectedSettingsHubItem = (g_selectedSettingsHubItem + 1) % 4; }

inline void selectPrevRoomNameItem() { g_selectedRoomNameItem = (g_selectedRoomNameItem + ROOM_COUNT - 1) % ROOM_COUNT; }
inline void selectNextRoomNameItem() { g_selectedRoomNameItem = (g_selectedRoomNameItem + 1) % ROOM_COUNT; }

inline void selectPrevIconsMenuItem() {
  g_selectedIconsMenuItem = (g_selectedIconsMenuItem + ICONS_MENU_COUNT - 1) % ICONS_MENU_COUNT;
}

inline void selectNextIconsMenuItem() {
  g_selectedIconsMenuItem = (g_selectedIconsMenuItem + 1) % ICONS_MENU_COUNT;
}

inline uint32_t currentAutoHomeTimeoutMs() {
  switch (g_autoHomeSetting) {
    case 0:  return 15000;
    case 1:  return 30000;
    case 2:  return 60000;
    default: return 0;
  }
}

inline void applyCurrentScreenSetting() {
  switch (g_selectedSettingsItem) {
    case SCREEN_SETTING_BRIGHTNESS:
      g_brightnessSetting = (g_brightnessSetting + 1) % 3;
      applyBacklightLevel();
      break;

    case SCREEN_SETTING_THEME:
      g_themeSetting = (g_themeSetting + 1) % 3;
      applyTheme();
      g_prevTempHome = -999.0f;
      g_prevTempOut  = -999.0f;
      g_prevDateStr  = "";
      g_prevDayStr   = "";
      g_prevTimeHH   = "";
      g_prevTimeMM   = "";
      break;

    case SCREEN_SETTING_AUTO_HOME:
      g_autoHomeSetting = (g_autoHomeSetting + 1) % 4;
      break;
  }

  saveUserSettings();
  forceFullDynamicRefresh();
}

inline void enterSelectedRoom() {
  switch (g_selectedMenuItem) {
    case 0: goToPage(PAGE_ROOM_SALON);     break;
    case 1: goToPage(PAGE_ROOM_GABINET);   break;
    case 2: goToPage(PAGE_ROOM_SYPIALNIA); break;
    case 3: goToPage(PAGE_SETTINGS);       return;
  }

  loadCurrentRoomDataFromBuffers();
}

inline void openSelectedSettingsTile() {
  switch (g_selectedSettingsHubItem) {
    case 0:
      goToPage(PAGE_SETTINGS_INFO);
      break;
    case 1:
      goToPage(PAGE_SETTINGS_SCREEN);
      break;
    case 2:
      goToPage(PAGE_SETTINGS_ROOMS);
      break;
    default:
      goToPage(PAGE_SETTINGS_ICONS_MENU);
      break;
  }
}

inline void openSelectedIconsTile() {
  switch (g_selectedIconsMenuItem) {
    case ICONS_MENU_MAIN:
      goToPage(PAGE_SETTINGS_ICONS_MAIN);
      break;
    case ICONS_MENU_DAY:
      goToPage(PAGE_SETTINGS_ICONS_DAY);
      break;
    default:
      goToPage(PAGE_SETTINGS_ICONS_NIGHT);
      break;
  }
}

inline int charsetIndexOf(char c) {
  const String chars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
  const int idx = chars.indexOf(c);
  return idx >= 0 ? idx : 0;
}

inline void rotateRoomNameChar(int delta) {
  String chars = " ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
  String value = g_roomNames[g_editRoomIndex];

  while ((int)value.length() < ROOM_NAME_MAX_LEN) value += ' ';

  int idx = charsetIndexOf(value[g_editCharIndex]);
  idx = (idx + delta + chars.length()) % chars.length();
  value.setCharAt(g_editCharIndex, chars[idx]);
  value.trim();

  g_roomNames[g_editRoomIndex] = value;
  syncMenuLabelsWithRoomNames();

  if (roomIndexFromPage(g_currentPage) == g_editRoomIndex) {
    g_roomTitle = roomDisplayName(g_editRoomIndex);
  }

  forceFullDynamicRefresh();
}

inline void handleUiIdleHomeReturn() {
  if (g_currentPage == PAGE_HOME) return;

  const uint32_t timeoutMs = currentAutoHomeTimeoutMs();
  if (timeoutMs == 0) return;

  if (millis() - g_lastUserActionMs >= timeoutMs) {
    g_selectedMenuItem = 0;
    g_selectedControlTile = 0;
    g_selectedSettingsItem = 0;
    g_selectedSettingsHubItem = 0;
    g_selectedRoomNameItem = 0;
    g_selectedIconsMenuItem = 0;

    goToPage(PAGE_HOME);
    markUserActivity();
  }
}

inline void handleButtonUp() {
  markUserActivity();

  if (g_currentPage == PAGE_MENU) {
    int oldIdx = g_selectedMenuItem;
    selectPrevMenuItem();
    if (oldIdx != g_selectedMenuItem) g_uiSelectFlashMs = millis();

  } else if (isRoomPage(g_currentPage)) {
    int oldIdx = g_selectedControlTile;
    selectPrevControlTile();
    if (oldIdx != g_selectedControlTile) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS_SCREEN) {
    int oldIdx = g_selectedSettingsItem;
    selectPrevSettingsItem();
    if (oldIdx != g_selectedSettingsItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS) {
    int oldIdx = g_selectedSettingsHubItem;
    selectPrevSettingsHubItem();
    if (oldIdx != g_selectedSettingsHubItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS_ROOMS) {
    selectPrevRoomNameItem();

  } else if (g_currentPage == PAGE_SETTINGS_ICONS_MENU) {
    int oldIdx = g_selectedIconsMenuItem;
    selectPrevIconsMenuItem();
    if (oldIdx != g_selectedIconsMenuItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_ROOM_NAME_EDIT) {
    rotateRoomNameChar(-1);
  }
}

inline void handleButtonDown() {
  markUserActivity();

  if (g_currentPage == PAGE_MENU) {
    int oldIdx = g_selectedMenuItem;
    selectNextMenuItem();
    if (oldIdx != g_selectedMenuItem) g_uiSelectFlashMs = millis();

  } else if (isRoomPage(g_currentPage)) {
    int oldIdx = g_selectedControlTile;
    selectNextControlTile();
    if (oldIdx != g_selectedControlTile) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS_SCREEN) {
    int oldIdx = g_selectedSettingsItem;
    selectNextSettingsItem();
    if (oldIdx != g_selectedSettingsItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS) {
    int oldIdx = g_selectedSettingsHubItem;
    selectNextSettingsHubItem();
    if (oldIdx != g_selectedSettingsHubItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_SETTINGS_ROOMS) {
    selectNextRoomNameItem();

  } else if (g_currentPage == PAGE_SETTINGS_ICONS_MENU) {
    int oldIdx = g_selectedIconsMenuItem;
    selectNextIconsMenuItem();
    if (oldIdx != g_selectedIconsMenuItem) g_uiSelectFlashMs = millis();

  } else if (g_currentPage == PAGE_ROOM_NAME_EDIT) {
    rotateRoomNameChar(1);
  }
}

inline void handleButtonOk() {
  markUserActivity();

  if (g_currentPage == PAGE_HOME) {
    goToPage(PAGE_MENU);

  } else if (g_currentPage == PAGE_MENU) {
    enterSelectedRoom();

  } else if (isRoomPage(g_currentPage)) {
    sendRoomControl();

  } else if (g_currentPage == PAGE_SETTINGS) {
    openSelectedSettingsTile();

  } else if (g_currentPage == PAGE_SETTINGS_ICONS_MENU) {
    openSelectedIconsTile();

  } else if (g_currentPage == PAGE_SETTINGS_SCREEN) {
    applyCurrentScreenSetting();

  } else if (g_currentPage == PAGE_SETTINGS_ROOMS) {
    g_editRoomIndex = g_selectedRoomNameItem;
    g_editCharIndex = 0;
    goToPage(PAGE_ROOM_NAME_EDIT);

  } else if (g_currentPage == PAGE_ROOM_NAME_EDIT) {
    g_editCharIndex = (g_editCharIndex + 1) % ROOM_NAME_MAX_LEN;
    forceFullDynamicRefresh();
  }
}

inline void handleButtonBack() {
  markUserActivity();

  if (g_currentPage == PAGE_MENU) {
    goToPage(PAGE_HOME);

  } else if (isRoomPage(g_currentPage) || g_currentPage == PAGE_SETTINGS) {
    goToPage(PAGE_MENU);

  } else if (g_currentPage == PAGE_SETTINGS_SCREEN ||
             g_currentPage == PAGE_SETTINGS_INFO ||
             g_currentPage == PAGE_SETTINGS_ROOMS ||
             g_currentPage == PAGE_SETTINGS_ICONS_MENU) {
    goToPage(PAGE_SETTINGS);

  } else if (g_currentPage == PAGE_SETTINGS_ICONS_MAIN ||
             g_currentPage == PAGE_SETTINGS_ICONS_DAY ||
             g_currentPage == PAGE_SETTINGS_ICONS_NIGHT) {
    goToPage(PAGE_SETTINGS_ICONS_MENU);

  } else if (g_currentPage == PAGE_ROOM_NAME_EDIT) {
    saveUserSettings();
    syncMenuLabelsWithRoomNames();
    goToPage(PAGE_SETTINGS_ROOMS);
  }
}

inline void handleEncoderInput() {
  static int lastUp = HIGH;
  static int lastDown = HIGH;
  static int lastOk = HIGH;
  static int lastBack = HIGH;
  static uint32_t lastInputMs = 0;

  int up = digitalRead(BTN_UP_PIN);
  int down = digitalRead(BTN_DOWN_PIN);
  int ok = digitalRead(BTN_OK_PIN);
  int back = digitalRead(BTN_BACK_PIN);

  if (millis() - lastInputMs >= INPUT_DEBOUNCE_MS) {
    if (lastUp == HIGH && up == LOW) {
      handleButtonUp();
      lastInputMs = millis();
    }

    if (lastDown == HIGH && down == LOW) {
      handleButtonDown();
      lastInputMs = millis();
    }

    if (lastOk == HIGH && ok == LOW) {
      handleButtonOk();
      lastInputMs = millis();
    }

    if (lastBack == HIGH && back == LOW) {
      handleButtonBack();
      lastInputMs = millis();
    }
  }

  lastUp = up;
  lastDown = down;
  lastOk = ok;
  lastBack = back;

  handleUiIdleHomeReturn();
}
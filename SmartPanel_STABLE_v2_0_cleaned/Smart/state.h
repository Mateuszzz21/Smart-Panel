#pragma once
#include "config.h"

enum PageId {
  PAGE_HOME = 0,
  PAGE_MENU,
  PAGE_ROOM_SALON,
  PAGE_ROOM_GABINET,
  PAGE_ROOM_SYPIALNIA,
  PAGE_SETTINGS,
  PAGE_SETTINGS_INFO,
  PAGE_SETTINGS_SCREEN,
  PAGE_SETTINGS_ROOMS,
  //PAGE_SETTINGS_ICONS,
  PAGE_SETTINGS_ICONS_MENU,
  PAGE_SETTINGS_ICONS_MAIN,
  PAGE_SETTINGS_ICONS_DAY,
  PAGE_SETTINGS_ICONS_NIGHT,
  PAGE_ROOM_NAME_EDIT
};

enum ScreenSettingsItem {
  SCREEN_SETTING_BRIGHTNESS = 0,
  SCREEN_SETTING_THEME,
  SCREEN_SETTING_AUTO_HOME,
  SCREEN_SETTING_COUNT
};

enum IconsMenuIndex {
  ICONS_MENU_MAIN = 0,
  ICONS_MENU_DAY,
  ICONS_MENU_NIGHT,
  ICONS_MENU_COUNT
};

extern int g_iconsMenuIndex;

enum WiFiUiState {
  WIFI_UI_DISCONNECTED = 0,
  WIFI_UI_CONNECTING,
  WIFI_UI_CONNECTED
};

enum TuyaUiState {
  TUYA_UI_DISCONNECTED = 0,
  TUYA_UI_CONNECTING,
  TUYA_UI_CONNECTED
};

struct Rect {
  int x;
  int y;
  int w;
  int h;
};

struct MenuItem {
  String label;
};

struct ControlTile {
  String label;
};

extern bool g_wifiOk;
extern bool g_tuyaOk;
extern bool g_forceFullDynamic;

extern WiFiUiState g_wifiUiState;
extern WiFiUiState g_prevWifiUiState;

extern TuyaUiState g_tuyaUiState;
extern TuyaUiState g_prevTuyaUiState;

extern String g_timeHH;
extern String g_timeMM;
extern bool   g_colonVisible;

extern String g_dateStr;
extern String g_dayStr;

extern float g_tempHome;
extern float g_tempOut;

extern float g_roomTemp;
extern float g_roomHum;

extern float g_roomTemps[3];
extern float g_roomHums[3];
extern uint16_t g_roomBattMv[3];
extern uint8_t  g_roomBattPct[3];
extern uint8_t  g_roomFlags[3];
extern uint32_t g_roomLastSeq[3];

extern uint16_t g_outBattMv;
extern uint8_t  g_outBattPct;
extern bool     g_outLowBat;
extern uint8_t  g_outFlags;

extern bool g_hasSavedTempHome;
extern bool g_hasSavedTempOut;
extern bool g_hasSavedRoomClimate[3];

extern bool  g_tuyaSwitchStates[2];
extern bool  g_prevTuyaSwitchStates[2];

extern String g_prevTimeHH;
extern String g_prevTimeMM;
extern bool   g_prevColonVisible;
extern String g_prevDateStr;
extern String g_prevDayStr;
extern bool   g_prevWifiOk;
extern bool   g_prevTuyaOk;
extern float  g_prevTempHome;
extern float  g_prevTempOut;
extern float  g_prevRoomTemp;
extern float  g_prevRoomHum;
extern uint16_t g_prevRoomBattMv;
extern uint8_t  g_prevRoomBattPct;
extern uint8_t  g_prevRoomFlags;
extern bool g_prevHomeGabinetLowBat;
extern bool g_prevHomeSalonLowBat;
extern bool g_prevHomeOutLowBat;
extern bool g_prevHomeAnyLowBat;

extern PageId g_currentPage;
extern PageId g_prevPage;

extern int g_selectedMenuItem;
extern int g_prevSelectedMenuItem;

extern int g_selectedControlTile;
extern int g_prevSelectedControlTile;

extern int g_selectedSettingsItem;
extern int g_prevSelectedSettingsItem;
extern int g_selectedSettingsHubItem;
extern int g_prevSelectedSettingsHubItem;
extern int g_selectedRoomNameItem;
extern int g_prevSelectedRoomNameItem;
extern int g_editRoomIndex;
extern int g_editCharIndex;
extern uint8_t g_brightnessSetting;
extern uint8_t g_themeSetting;
extern uint8_t g_autoHomeSetting;

extern uint32_t g_uiSelectFlashMs;

extern MenuItem g_menuItems[4];
extern Rect g_menuRects[4];

extern ControlTile g_controlTiles[2];
extern Rect g_controlRects[2];

extern String g_roomNames[3];
extern String g_roomTitle;
extern String g_prevEditRoomValue;
extern int g_prevEditCharIndex;

extern uint32_t g_lastUserActionMs;
extern uint32_t g_lastUdpPacketMs;

extern uint16_t g_colBg;
extern uint16_t g_colWhite;
extern uint16_t g_colTextMuted;
extern uint16_t g_colBorder;
extern uint16_t g_colBorderActive;
extern uint16_t g_colAccent;
extern uint16_t g_colCardActive;
extern uint16_t g_colError;
extern uint16_t g_colText;
extern uint16_t g_colCard;
extern uint16_t g_colOk;

void forceFullDynamicRefresh();
void syncCurrentRoomDataForActivePage();
void loadCurrentRoomDataFromBuffers();
void syncMenuLabelsWithRoomNames();

void saveUserSettings();
void loadUserSettings();
void applyTheme();

void saveClimateStateToNvs();
void loadClimateStateFromNvs();
extern uint8_t g_weatherNowCode;
extern uint8_t g_weatherPlus4hCode;
extern float   g_weatherNowTemp;
extern float   g_weatherPlus4hTemp;

extern uint8_t g_prevWeatherNowCode;
extern uint8_t g_prevWeatherPlus4hCode;

extern int g_selectedIconsMenuItem;
extern int g_prevSelectedIconsMenuItem;
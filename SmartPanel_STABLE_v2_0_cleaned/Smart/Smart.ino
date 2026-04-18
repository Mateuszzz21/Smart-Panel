#include "config.h"
#include "objects.h"
#include "state.h"
#include "wifi_manager.h"
#include "time_utils.h"
#include "ui_layout.h"
#include "ui_draw.h"
#include "pages_home.h"
#include "room_config.h"
#include "room_logic.h"
#include "tuya_client.h"
#include "input_encoder.h"
#include "panel_udp.h"
#include "panel_sensors.h"
#include "panel_weather.h"
#include "api_server.h"
#include "ui_draw.h"

TFT_eSPI tft = TFT_eSPI();
WiFiUDP udpDiscovery;
WiFiUDP udpRoomData;
WebServer apiServer(API_PORT);
Preferences g_prefs;

// ============================
// GLOBAL STATE
// ============================
bool g_wifiOk = false;
bool g_tuyaOk = false;
bool g_prevWifiOk = false;
bool g_prevTuyaOk = false;
bool g_forceFullDynamic = true;

WiFiUiState g_wifiUiState = WIFI_UI_DISCONNECTED;
WiFiUiState g_prevWifiUiState = WIFI_UI_DISCONNECTED;

TuyaUiState g_tuyaUiState = TUYA_UI_DISCONNECTED;
TuyaUiState g_prevTuyaUiState = TUYA_UI_DISCONNECTED;

String g_timeHH = "--";
String g_timeMM = "--";
bool   g_colonVisible = true;

String g_dateStr = "--.--.----";
String g_dayStr  = "---";

float g_tempHome = 0.0f;
float g_tempOut  = 0.0f;

uint16_t g_outBattMv = 0;
uint8_t  g_outBattPct = 0;
bool     g_outLowBat = false;


float g_roomTemp = 0.0f;
float g_roomHum  = 0.0f;

float g_roomTemps[3] = {22.6f, 23.1f, 21.4f};
float g_roomHums[3]  = {47.0f, 43.0f, 51.0f};
uint16_t g_roomBattMv[3]  = {0, 0, 0};
uint8_t  g_roomBattPct[3] = {0, 0, 0};
uint8_t  g_roomFlags[3]   = {0, 0, 0};
uint32_t g_roomLastSeq[3] = {0, 0, 0};

bool g_hasSavedTempHome = false;
bool g_hasSavedTempOut = false;
bool g_hasSavedRoomClimate[3] = {false, false, false};

bool  g_tuyaSwitchStates[2] = {false, false};
bool  g_prevTuyaSwitchStates[2] = {true, true};

String g_prevTimeHH = "";
String g_prevTimeMM = "";
bool   g_prevColonVisible = true;
String g_prevDateStr = "";
String g_prevDayStr  = "";

float   g_prevTempHome = -999.0f;
float   g_prevTempOut  = -999.0f;
float   g_prevRoomTemp = -999.0f;
float   g_prevRoomHum  = -999.0f;
uint16_t g_prevRoomBattMv  = 0xFFFF;
uint8_t  g_prevRoomBattPct = 255;
uint8_t  g_prevRoomFlags   = 255;

uint8_t g_outFlags = 0;
bool g_prevHomeGabinetLowBat = false;
bool g_prevHomeSalonLowBat = false;
bool g_prevHomeOutLowBat = false;
bool g_prevHomeAnyLowBat = false;

uint32_t g_uiSelectFlashMs = 0;

PageId g_currentPage = PAGE_HOME;
PageId g_prevPage    = PAGE_HOME;

int g_selectedMenuItem = 0;
int g_prevSelectedMenuItem = -1;

int g_selectedControlTile = 0;
int g_prevSelectedControlTile = -1;

int g_selectedSettingsItem = 0;
int g_prevSelectedSettingsItem = -1;
int g_selectedSettingsHubItem = 0;
int g_prevSelectedSettingsHubItem = -1;
int g_selectedRoomNameItem = 0;
int g_prevSelectedRoomNameItem = -1;
int g_editRoomIndex = 0;
int g_editCharIndex = 0;

uint8_t g_brightnessSetting = 2;
uint8_t g_themeSetting = 0;
uint8_t g_autoHomeSetting = 1;
int g_selectedIconsMenuItem = 0;
int g_prevSelectedIconsMenuItem = -1;
String g_prevEditRoomValue = "";
int g_prevEditCharIndex = -1;

uint8_t g_weatherNowCode = 255;
uint8_t g_weatherPlus4hCode = 255;
float g_weatherNowTemp = NAN;
float g_weatherPlus4hTemp = NAN;

uint8_t g_prevWeatherNowCode = 255;
uint8_t g_prevWeatherPlus4hCode = 255;


MenuItem g_menuItems[4] = {
  {"Salon"},
  {"Gabinet"},
  {"Sypialnia"},
  {"Ustawienia"}
};

Rect g_menuRects[4];

ControlTile g_controlTiles[2] = {
  {"ON"},
  {"OFF"}
};

Rect g_controlRects[2];

String g_roomTitle = "SALON";
String g_roomNames[3] = {"SALON", "GABINET", "SYPIALNIA"};
uint32_t g_lastUserActionMs = 0;
uint32_t g_lastUdpPacketMs = 0;

uint16_t g_colBg           = COL_BG;
uint16_t g_colText         = COL_TEXT;
uint16_t g_colTextMuted    = COL_TEXT_MUTED;
uint16_t g_colCard         = COL_CARD;
uint16_t g_colCardActive   = COL_CARD_ACTIVE;
uint16_t g_colBorder       = COL_BORDER;
uint16_t g_colBorderActive = COL_BORDER_ACTIVE;
uint16_t g_colOk           = COL_OK;
uint16_t g_colError        = COL_ERROR;
uint16_t g_colAccent       = COL_ACCENT;
uint16_t g_colWhite        = COL_WHITE;

// ============================
// COLOR HELPERS
// ============================
inline uint16_t rgb565From888(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

inline void rgb888From565(uint16_t c, uint8_t& r, uint8_t& g, uint8_t& b) {
  r = ((c >> 11) & 0x1F) << 3;
  g = ((c >> 5)  & 0x3F) << 2;
  b = (c & 0x1F) << 3;
}

inline uint16_t lighten565(uint16_t c, uint8_t amount) {
  uint8_t r, g, b;
  rgb888From565(c, r, g, b);
  r = r + ((255 - r) * amount) / 255;
  g = g + ((255 - g) * amount) / 255;
  b = b + ((255 - b) * amount) / 255;
  return rgb565From888(r, g, b);
}

inline uint16_t darken565(uint16_t c, uint8_t amount) {
  uint8_t r, g, b;
  rgb888From565(c, r, g, b);
  r = (r * (255 - amount)) / 255;
  g = (g * (255 - amount)) / 255;
  b = (b * (255 - amount)) / 255;
  return rgb565From888(r, g, b);
}

// ============================
// SETTINGS / NVS
// ============================
inline String sanitizeRoomName(const String& src) {
  String out;
  out.reserve(ROOM_NAME_MAX_LEN);

  for (size_t i = 0; i < src.length() && out.length() < ROOM_NAME_MAX_LEN; i++) {
    char c = src[i];
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';

    if ((c >= 'A' && c <= 'Z') ||
        (c >= '0' && c <= '9') ||
        c == ' ' || c == '-' || c == '_') {
      out += c;
    }
  }

  out.trim();
  if (out.length() == 0) out = "POKOJ";
  return out;
}

void saveUserSettings() {
  if (!g_prefs.begin(NVS_NAMESPACE, false)) return;

  g_prefs.putUChar("bright", g_brightnessSetting);
  g_prefs.putUChar("theme", g_themeSetting);
  g_prefs.putUChar("autohome", g_autoHomeSetting);
  g_prefs.putString("room0", g_roomNames[0]);
  g_prefs.putString("room1", g_roomNames[1]);
  g_prefs.putString("room2", g_roomNames[2]);

  g_prefs.end();
}

void loadUserSettings() {
  if (!g_prefs.begin(NVS_NAMESPACE, true)) return;

  g_brightnessSetting = g_prefs.getUChar("bright", g_brightnessSetting);
  g_themeSetting      = g_prefs.getUChar("theme", g_themeSetting);
  g_autoHomeSetting   = g_prefs.getUChar("autohome", g_autoHomeSetting);

  g_roomNames[0] = sanitizeRoomName(g_prefs.getString("room0", ROOM_NAMES[0]));
  g_roomNames[1] = sanitizeRoomName(g_prefs.getString("room1", ROOM_NAMES[1]));
  g_roomNames[2] = sanitizeRoomName(g_prefs.getString("room2", ROOM_NAMES[2]));

  g_prefs.end();
  syncMenuLabelsWithRoomNames();
}

void saveClimateStateToNvs() {
  if (!g_prefs.begin(NVS_NAMESPACE, false)) return;

  g_prefs.putBool("th_ok", true);
  g_prefs.putFloat("thome", g_tempHome);

  g_prefs.putBool("to_ok", true);
  g_prefs.putFloat("tout", g_tempOut);

  g_prefs.putUShort("obmv", g_outBattMv);
  g_prefs.putUChar("obpc", g_outBattPct);
  g_prefs.putBool("olow", g_outLowBat);
  g_prefs.putUChar("oflg", g_outFlags);

  for (int i = 0; i < 3; i++) {
    String keyOk = "r" + String(i) + "ok";
    String keyT  = "rt" + String(i);
    String keyH  = "rh" + String(i);

    g_prefs.putBool(keyOk.c_str(), true);
    g_prefs.putFloat(keyT.c_str(), g_roomTemps[i]);
    g_prefs.putFloat(keyH.c_str(), g_roomHums[i]);
  }

  g_prefs.end();
}

void loadClimateStateFromNvs() {
  if (!g_prefs.begin(NVS_NAMESPACE, true)) return;

  g_hasSavedTempHome = g_prefs.getBool("th_ok", false);
  if (g_hasSavedTempHome) {
    g_tempHome = g_prefs.getFloat("thome", g_tempHome);
  }

  g_hasSavedTempOut = g_prefs.getBool("to_ok", false);
  if (g_hasSavedTempOut) {
    g_tempOut = g_prefs.getFloat("tout", g_tempOut);
    g_outBattMv  = g_prefs.getUShort("obmv", 0);
    g_outBattPct = g_prefs.getUChar("obpc", 0);
    g_outLowBat  = g_prefs.getBool("olow", false);
    g_outFlags   = g_prefs.getUChar("oflg", 0);
  }

  for (int i = 0; i < 3; i++) {
    String keyOk = "r" + String(i) + "ok";
    String keyT  = "rt" + String(i);
    String keyH  = "rh" + String(i);

    g_hasSavedRoomClimate[i] = g_prefs.getBool(keyOk.c_str(), false);
    if (g_hasSavedRoomClimate[i]) {
      g_roomTemps[i] = g_prefs.getFloat(keyT.c_str(), g_roomTemps[i]);
      g_roomHums[i]  = g_prefs.getFloat(keyH.c_str(), g_roomHums[i]);
    }
  }

  g_prefs.end();
}

// ============================
// THEME
// ============================
inline void applyTheme() {
  uint16_t baseBg;
  uint16_t baseText;
  uint16_t baseTextMuted;
  uint16_t baseCard;
  uint16_t baseCardActive;
  uint16_t baseBorder;
  uint16_t baseBorderActive;
  uint16_t baseOk;
  uint16_t baseError;
  uint16_t baseAccent;
  uint16_t baseWhite;

  switch (g_themeSetting) {
    case 0:
      baseBg = TFT_BLACK;
      baseText = 0xFFFF;
      baseTextMuted = 0xAD55;
      baseCard = 0x2104;
      baseCardActive = 0x39C7;
      baseBorder = 0x4208;
      baseBorderActive = 0xFB04;
      baseOk = 0x8660;
      baseError = 0xF800;
      baseAccent = 0xFA40;
      baseWhite = 0xFFFF;
      break;

    case 1:
      baseBg = 0x10A3;
      baseText = 0xFFFF;
      baseTextMuted = 0xC638;
      baseCard = 0x2147;
      baseCardActive = 0x328D;
      baseBorder = 0x4B55;
      baseBorderActive = 0xFD20;
      baseOk = 0x8660;
      baseError = 0xF800;
      baseAccent = 0x051D;
      baseWhite = 0xFFFF;
      break;

    default:
      baseBg = 0x2124;
      baseText = 0xFFFF;
      baseTextMuted = 0xBDF7;
      baseCard = 0x3186;
      baseCardActive = 0x4A49;
      baseBorder = 0x632C;
      baseBorderActive = 0xFD20;
      baseOk = 0x8660;
      baseError = 0xF800;
      baseAccent = 0x7BEF;
      baseWhite = 0xFFFF;
      break;
  }

  switch (g_brightnessSetting) {
    case 0:
      g_colBg = lighten565(baseBg, 12);
      g_colText = darken565(baseText, 58);
      g_colTextMuted = darken565(baseTextMuted, 72);
      g_colCard = lighten565(baseCard, 10);
      g_colCardActive = lighten565(baseCardActive, 6);
      g_colBorder = darken565(baseBorder, 32);
      g_colBorderActive = darken565(baseBorderActive, 10);
      g_colOk = darken565(baseOk, 18);
      g_colError = darken565(baseError, 10);
      g_colAccent = darken565(baseAccent, 22);
      g_colWhite = darken565(baseWhite, 52);
      break;

    case 1:
      g_colBg = baseBg;
      g_colText = baseText;
      g_colTextMuted = baseTextMuted;
      g_colCard = baseCard;
      g_colCardActive = baseCardActive;
      g_colBorder = baseBorder;
      g_colBorderActive = baseBorderActive;
      g_colOk = baseOk;
      g_colError = baseError;
      g_colAccent = baseAccent;
      g_colWhite = baseWhite;
      break;

    default:
      g_colBg = darken565(baseBg, 18);
      g_colText = baseText;
      g_colTextMuted = lighten565(baseTextMuted, 14);
      g_colCard = darken565(baseCard, 10);
      g_colCardActive = lighten565(baseCardActive, 10);
      g_colBorder = lighten565(baseBorder, 8);
      g_colBorderActive = lighten565(baseBorderActive, 6);
      g_colOk = lighten565(baseOk, 8);
      g_colError = baseError;
      g_colAccent = lighten565(baseAccent, 18);
      g_colWhite = baseWhite;
      break;
  }

  applyBacklightLevel();
}

inline void printLittleFSInfo() {
  size_t total = LittleFS.totalBytes();
  size_t used  = LittleFS.usedBytes();
  size_t freeb = (total >= used) ? (total - used) : 0;

  Serial.println();
  Serial.println("===== LITTLEFS INFO =====");
  Serial.print("Total: ");
  Serial.print(total);
  Serial.println(" B");

  Serial.print("Used : ");
  Serial.print(used);
  Serial.println(" B");

  Serial.print("Free : ");
  Serial.print(freeb);
  Serial.println(" B");

  Serial.print("Total: ");
  Serial.print(total / 1024.0, 1);
  Serial.println(" KB");

  Serial.print("Used : ");
  Serial.print(used / 1024.0, 1);
  Serial.println(" KB");

  Serial.print("Free : ");
  Serial.print(freeb / 1024.0, 1);
  Serial.println(" KB");

  if (total > 0) {
    float pct = (100.0f * used) / (float)total;
    Serial.print("Usage: ");
    Serial.print(pct, 1);
    Serial.println(" %");
  }

  Serial.println("=========================");
  Serial.println();
}

#include "esp_heap_caps.h"

void printMemoryStats() {
  Serial.println("===== MEMORY =====");

  Serial.print("Free heap: ");
  Serial.println(ESP.getFreeHeap());

  Serial.print("Min free heap: ");
  Serial.println(ESP.getMinFreeHeap());

  Serial.print("Max alloc heap: ");
  Serial.println(ESP.getMaxAllocHeap());

  Serial.print("Largest free block: ");
  Serial.println(heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));

  Serial.println("==================");
}

// ============================
// SETUP / LOOP
// ============================
void setup() {
  Serial.begin(115200);
  delay(300);

 if (!LittleFS.begin(true)) {
  Serial.println("[FS] LittleFS mount fail");
} else {
  Serial.println("[FS] LittleFS OK");
  printLittleFSInfo();
}


  loadUserSettings();
  loadClimateStateFromNvs();
  applyTheme();
  initDisplay();
  drawBootScreen("BOOT...");

  initUiLayout();
  initEncoderInput();
  initPanelSensors();
  syncMenuLabelsWithRoomNames();
  g_lastUserActionMs = millis();

  drawBootScreen("WIFI...");
  connectWiFi();

  drawBootScreen("NTP...");
  configLocalTime();

  initPanelUdp();

  drawBootScreen("TUYA...");
  initTuya();

  drawBootScreen("API...");
  initApiServer();

  drawCurrentPageStatic();
  forceFullDynamicRefresh();

  initLowBatLed();
  updateLowBatLed();
  printMemoryStats();
}

void loop() {
  handleWiFi();
  handleTuya();
  handlePanelSensors();
  handleInternetWeather();
  updateClockCache();
  handleEncoderInput();
  handlePanelUdp();
  handleApiServer();
  syncCurrentRoomDataForActivePage();
  drawCurrentPageDynamic();
  updateLowBatLed();
  static unsigned long lastMem = 0;

if (millis() - lastMem > 5000) {
  printMemoryStats();
  lastMem = millis();
}
  delay(10);
}
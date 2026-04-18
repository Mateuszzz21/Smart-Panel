#pragma once
#include "api_json.h"
#include "state.h"
#include "ui_draw_base.h"

void saveUserSettings();
void applyTheme();

inline const char* apiSettingsThemeName() {
  switch (g_themeSetting) {
    case 0: return "warm";
    case 1: return "blue";
    default: return "graphite";
  }
}

inline uint16_t apiSettingsAutoHomeSec() {
  switch (g_autoHomeSetting) {
    case 0: return 15;
    case 1: return 30;
    case 2: return 60;
    default: return 0;
  }
}

inline void apiSendSettingsData() {
  DynamicJsonDocument doc(256);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");
  data["brightnessLevel"] = g_brightnessSetting;
  data["themeLevel"] = g_themeSetting;
  data["autoReturnLevel"] = g_autoHomeSetting;
  data["theme"] = apiSettingsThemeName();
  data["autoReturnSec"] = apiSettingsAutoHomeSec();
  apiSendJsonDocument(200, doc);
}

inline int apiThemeIndexFromName(const String& theme) {
  if (theme == "warm") return 0;
  if (theme == "blue") return 1;
  if (theme == "graphite") return 2;
  return -1;
}

inline int apiAutoHomeLevelFromSec(int sec) {
  if (sec == 15) return 0;
  if (sec == 30) return 1;
  if (sec == 60) return 2;
  if (sec == 0) return 3;
  return -1;
}

inline int apiBrightnessLevelFromPwm(int value) {
  if (value <= 90) return 0;
  if (value <= 190) return 1;
  return 2;
}

inline void apiHandleSettings() {
  if (!apiRequireToken()) return;

  if (apiServer.method() == HTTP_GET) {
    apiSendSettingsData();
    return;
  }

  if (apiServer.method() != HTTP_POST) {
    apiSendError(404, "not_found");
    return;
  }

  DynamicJsonDocument body(256);
  if (!apiParseBody(body)) {
    apiSendError(400, "invalid_json");
    return;
  }

  bool changed = false;

  if (body["brightnessLevel"].is<int>()) {
    int v = body["brightnessLevel"].as<int>();
    if (v >= 0 && v <= 2) {
      g_brightnessSetting = (uint8_t)v;
      changed = true;
    }
  } else if (body["brightness"].is<int>()) {
    int v = body["brightness"].as<int>();
    if (v >= 0 && v <= 255) {
      g_brightnessSetting = (uint8_t)apiBrightnessLevelFromPwm(v);
      changed = true;
    }
  }

  if (body["themeLevel"].is<int>()) {
    int v = body["themeLevel"].as<int>();
    if (v >= 0 && v <= 2) {
      g_themeSetting = (uint8_t)v;
      changed = true;
    }
  } else if (body["theme"].is<const char*>()) {
    int v = apiThemeIndexFromName(String(body["theme"].as<const char*>()));
    if (v >= 0) {
      g_themeSetting = (uint8_t)v;
      changed = true;
    }
  }

  if (body["autoReturnLevel"].is<int>()) {
    int v = body["autoReturnLevel"].as<int>();
    if (v >= 0 && v <= 3) {
      g_autoHomeSetting = (uint8_t)v;
      changed = true;
    }
  } else if (body["autoReturnSec"].is<int>()) {
    int v = apiAutoHomeLevelFromSec(body["autoReturnSec"].as<int>());
    if (v >= 0) {
      g_autoHomeSetting = (uint8_t)v;
      changed = true;
    }
  }

  if (!changed) {
    apiSendError(400, "no_valid_fields");
    return;
  }

  applyTheme();
  applyBacklightLevel();
  saveUserSettings();
  forceFullDynamicRefresh();
  apiSendSettingsData();
}

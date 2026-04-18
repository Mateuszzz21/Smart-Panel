#pragma once
#include "api_json.h"
#include "state.h"
#include "room_logic.h"
#include "room_config.h"
#include "config.h"
#include <math.h>
#include <time.h>

inline const char* apiThemeName() {
  switch (g_themeSetting) {
    case 0: return "warm";
    case 1: return "blue";
    default: return "graphite";
  }
}

inline uint8_t apiBrightnessPwmValue() {
  switch (g_brightnessSetting) {
    case 0: return TFT_BL_LEVEL_LOW;
    case 1: return TFT_BL_LEVEL_MID;
    default: return TFT_BL_LEVEL_HIGH;
  }
}

inline uint16_t apiAutoHomeSec() {
  switch (g_autoHomeSetting) {
    case 0: return 15;
    case 1: return 30;
    case 2: return 60;
    default: return 0;
  }
}

inline void apiAddRoomObject(JsonArray& rooms, int room) {
  JsonObject r = rooms.createNestedObject();
  r["id"] = (room == ROOM_SALON) ? "salon" : (room == ROOM_GABINET) ? "gabinet" : "sypialnia";
  r["name"] = roomDisplayName(room);
  r["temperature"] = g_roomTemps[room];
  r["humidity"] = g_roomHums[room];
  r["available"] = true;
  r["batteryMv"] = g_roomBattMv[room];
  r["batteryPct"] = g_roomBattPct[room];
  r["lowBat"] = (g_roomFlags[room] & 0x01) != 0;
  r["lastSeq"] = g_roomLastSeq[room];

  JsonObject controls = r.createNestedObject("controls");
  if (room == ROOM_SYPIALNIA) {
    controls["light1"] = g_tuyaSwitchStates[0];
    controls["light2"] = g_tuyaSwitchStates[1];
  } else {
    controls["light1"] = false;
    controls["light2"] = false;
  }
}

inline void apiHandleState() {
  if (!apiRequireToken()) return;

  DynamicJsonDocument doc(2048);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");

  JsonObject panel = data.createNestedObject("panel");
  panel["name"] = "Smart Panel";
  panel["version"] = FW_VERSION;
  panel["ip"] = g_wifiOk ? WiFi.localIP().toString() : String("0.0.0.0");
  panel["uptimeSec"] = millis() / 1000UL;

  JsonObject clock = data.createNestedObject("clock");
  clock["time"] = g_timeHH + ":" + g_timeMM;
  clock["date"] = g_dateStr;
  clock["weekday"] = g_dayStr;
  clock["epoch"] = (uint32_t)time(nullptr);

  JsonObject climate = data.createNestedObject("climate");
  climate["tempHome"] = isnan(g_tempHome) ? 0.0f : g_tempHome;
  climate["tempOut"] = isnan(g_tempOut) ? 0.0f : g_tempOut;
  climate["weatherNowCode"] = g_weatherNowCode;
  climate["weatherPlus4hCode"] = g_weatherPlus4hCode;
  climate["weatherNowTemp"] = isnan(g_weatherNowTemp) ? 0.0f : g_weatherNowTemp;
  climate["weatherPlus4hTemp"] = isnan(g_weatherPlus4hTemp) ? 0.0f : g_weatherPlus4hTemp;

  JsonObject status = data.createNestedObject("status");
  status["wifiOk"] = g_wifiOk;
  status["wifiRssi"] = g_wifiOk ? WiFi.RSSI() : 0;
  status["tuyaOk"] = g_tuyaOk;
  status["lastUdpPacketMsAgo"] = g_lastUdpPacketMs == 0 ? 0UL : millis() - g_lastUdpPacketMs;

  JsonArray rooms = data.createNestedArray("rooms");
  for (int i = 0; i < ROOM_COUNT; i++) {
    apiAddRoomObject(rooms, i);
  }

  JsonObject settings = data.createNestedObject("settings");
  settings["brightnessLevel"] = g_brightnessSetting;
  settings["brightnessPwm"] = apiBrightnessPwmValue();
  settings["autoReturnLevel"] = g_autoHomeSetting;
  settings["autoReturnSec"] = apiAutoHomeSec();
  settings["theme"] = apiThemeName();

  apiSendJsonDocument(200, doc);
}

#pragma once
#include "api_json.h"
#include "state.h"
#include <math.h>

inline void apiHandleHome() {
  if (!apiRequireToken()) return;

  DynamicJsonDocument doc(512);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");
  data["time"] = g_timeHH + ":" + g_timeMM;
  data["date"] = g_dateStr;
  data["weekday"] = g_dayStr;
  data["tempHome"] = isnan(g_tempHome) ? 0.0f : g_tempHome;
  data["tempOut"] = isnan(g_tempOut) ? 0.0f : g_tempOut;
  data["weatherNowCode"] = g_weatherNowCode;
  data["weatherPlus4hCode"] = g_weatherPlus4hCode;
  data["wifiOk"] = g_wifiOk;
  data["wifiRssi"] = g_wifiOk ? WiFi.RSSI() : 0;
  data["tuyaOk"] = g_tuyaOk;
  apiSendJsonDocument(200, doc);
}

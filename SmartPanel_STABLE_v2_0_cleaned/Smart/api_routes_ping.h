#pragma once
#include "api_json.h"
#include "config.h"
#include <WiFi.h>

inline void apiHandlePing() {
  if (!apiRequireToken()) return;

  DynamicJsonDocument doc(256);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");
  data["pong"] = true;
  data["ip"] = WiFi.isConnected() ? WiFi.localIP().toString() : String("0.0.0.0");
  data["version"] = FW_VERSION;
  apiSendJsonDocument(200, doc);
}

#pragma once
#include "api_auth.h"
#include <ArduinoJson.h>

inline void apiSendJsonDocument(int statusCode, DynamicJsonDocument& doc) {
  String out;
  serializeJson(doc, out);
  apiAddCommonHeaders();
  apiServer.send(statusCode, "application/json; charset=utf-8", out);
}

inline void apiSendOkOnly() {
  DynamicJsonDocument doc(96);
  doc["ok"] = true;
  apiSendJsonDocument(200, doc);
}

inline void apiSendError(int statusCode, const char* errorCode) {
  DynamicJsonDocument doc(160);
  doc["ok"] = false;
  doc["error"] = errorCode;
  apiSendJsonDocument(statusCode, doc);
}

inline bool apiRequireToken() {
  if (apiHandleCorsPreflight()) return false;
  if (!apiCheckToken()) {
    apiSendError(401, "unauthorized");
    return false;
  }
  return true;
}

inline bool apiParseBody(DynamicJsonDocument& doc) {
  const String body = apiServer.arg("plain");
  if (body.length() == 0) return false;
  return deserializeJson(doc, body) == DeserializationError::Ok;
}

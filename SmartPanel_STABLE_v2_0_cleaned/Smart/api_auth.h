#pragma once
#include "config.h"
#include <WebServer.h>

extern WebServer apiServer;

inline void apiAddCommonHeaders() {
  if (API_ALLOW_CORS) {
    apiServer.sendHeader("Access-Control-Allow-Origin", "*");
    apiServer.sendHeader("Access-Control-Allow-Headers", "Content-Type, X-Panel-Token");
    apiServer.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  }
  apiServer.sendHeader("Cache-Control", "no-store");
}

inline bool apiHandleCorsPreflight() {
  if (apiServer.method() != HTTP_OPTIONS) return false;
  apiAddCommonHeaders();
  apiServer.send(204);
  return true;
}

inline bool apiCheckToken() {
  if (!API_ENABLED) return false;
  const String token = apiServer.header("X-Panel-Token");
  return token == String(API_TOKEN);
}

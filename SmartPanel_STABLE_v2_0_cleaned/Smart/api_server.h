#pragma once
#include "config.h"
#include <WebServer.h>
#include "api_routes_ping.h"
#include "api_routes_home.h"
#include "api_routes_state.h"
#include "api_routes_rooms.h"
#include "api_routes_settings.h"

extern WebServer apiServer;

inline bool apiPathStartsWith(const String& value, const String& prefix) {
  return value.startsWith(prefix);
}

inline void apiHandleNotFound() {
  if (apiHandleCorsPreflight()) return;

  const String path = apiServer.uri();
  if (path == "/api/rooms" || path == "/api/rooms/" ||
      path == "/api/rooms/names" || apiPathStartsWith(path, "/api/rooms/")) {
    apiHandleRoomByPath();
    return;
  }

  apiSendError(404, "not_found");
}

inline void initApiServer() {
  if (!API_ENABLED) return;

  static const char* apiHeaderKeys[] = {"X-Panel-Token"};
  apiServer.collectHeaders(apiHeaderKeys, 1);

  apiServer.on("/api/ping", HTTP_ANY, apiHandlePing);
  apiServer.on("/api/home", HTTP_ANY, apiHandleHome);
  apiServer.on("/api/state", HTTP_ANY, apiHandleState);
  apiServer.on("/api/settings", HTTP_ANY, apiHandleSettings);

  apiServer.onNotFound(apiHandleNotFound);
  apiServer.begin();
  Serial.printf("[API] HTTP server start on port %u\n", (unsigned)API_PORT);
}

inline void handleApiServer() {
  if (!API_ENABLED) return;
  apiServer.handleClient();
}

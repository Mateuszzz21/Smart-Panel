#pragma once
#include "api_json.h"
#include "state.h"
#include "room_logic.h"
#include "room_config.h"
#include "tuya_client.h"
#include <math.h>

void saveUserSettings();

inline int apiRoomIndexFromId(const String& id) {
  if (id == "salon") return ROOM_SALON;
  if (id == "gabinet") return ROOM_GABINET;
  if (id == "sypialnia") return ROOM_SYPIALNIA;
  return -1;
}

inline String apiRoomIdFromIndex(int room) {
  if (room == ROOM_SALON) return "salon";
  if (room == ROOM_GABINET) return "gabinet";
  if (room == ROOM_SYPIALNIA) return "sypialnia";
  return "";
}

inline void apiSendRoomData(int room) {
  DynamicJsonDocument doc(512);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");
  data["id"] = apiRoomIdFromIndex(room);
  data["name"] = roomDisplayName(room);
  data["temperature"] = g_roomTemps[room];
  data["humidity"] = g_roomHums[room];
  data["available"] = true;
  data["batteryMv"] = g_roomBattMv[room];
  data["batteryPct"] = g_roomBattPct[room];
  data["lowBat"] = (g_roomFlags[room] & 0x01) != 0;
  data["lastSeq"] = g_roomLastSeq[room];
  data["light1"] = (room == ROOM_SYPIALNIA) ? g_tuyaSwitchStates[0] : false;
  data["light2"] = (room == ROOM_SYPIALNIA) ? g_tuyaSwitchStates[1] : false;
  apiSendJsonDocument(200, doc);
}

inline bool apiSetBedroomSwitch(int idx, bool value) {
  if (idx < 0 || idx > 1) return false;
  g_tuyaUiState = TUYA_UI_CONNECTING;
  const bool ok = tuyaSendBoolCommandRaw(BEDROOM_TUYA_SWITCH_CODES[idx], value);
  g_tuyaOk = ok;
  g_tuyaUiState = ok ? TUYA_UI_CONNECTED : TUYA_UI_DISCONNECTED;
  if (ok) {
    g_tuyaSwitchStates[idx] = value;
    g_forceFullDynamic = true;
  }
  return ok;
}

inline void apiHandleRoomsList() {
  if (!apiRequireToken()) return;

  DynamicJsonDocument doc(1024);
  doc["ok"] = true;
  JsonArray arr = doc.createNestedArray("data");
  for (int room = 0; room < ROOM_COUNT; room++) {
    JsonObject r = arr.createNestedObject();
    r["id"] = apiRoomIdFromIndex(room);
    r["name"] = roomDisplayName(room);
    r["temperature"] = g_roomTemps[room];
    r["humidity"] = g_roomHums[room];
    r["available"] = true;
    r["batteryMv"] = g_roomBattMv[room];
    r["batteryPct"] = g_roomBattPct[room];
    r["lowBat"] = (g_roomFlags[room] & 0x01) != 0;
    r["lastSeq"] = g_roomLastSeq[room];
    r["light1"] = (room == ROOM_SYPIALNIA) ? g_tuyaSwitchStates[0] : false;
    r["light2"] = (room == ROOM_SYPIALNIA) ? g_tuyaSwitchStates[1] : false;
  }
  apiSendJsonDocument(200, doc);
}

inline void apiHandleRoomNamesPost() {
  if (!apiRequireToken()) return;
  DynamicJsonDocument body(384);
  if (!apiParseBody(body)) {
    apiSendError(400, "invalid_json");
    return;
  }

  if (body["salon"].is<const char*>()) g_roomNames[ROOM_SALON] = String(body["salon"].as<const char*>());
  if (body["gabinet"].is<const char*>()) g_roomNames[ROOM_GABINET] = String(body["gabinet"].as<const char*>());
  if (body["sypialnia"].is<const char*>()) g_roomNames[ROOM_SYPIALNIA] = String(body["sypialnia"].as<const char*>());

  saveUserSettings();
  syncMenuLabelsWithRoomNames();
  loadCurrentRoomDataFromBuffers();
  forceFullDynamicRefresh();
  apiSendOkOnly();
}

inline void apiHandleRoomByPath() {
  if (!apiRequireToken()) return;

  String path = apiServer.uri();
  if (path == "/api/rooms" || path == "/api/rooms/") {
    apiHandleRoomsList();
    return;
  }
  if (path == "/api/rooms/names") {
    if (apiServer.method() == HTTP_POST) apiHandleRoomNamesPost();
    else apiSendError(404, "not_found");
    return;
  }

  const String prefix = "/api/rooms/";
  if (!path.startsWith(prefix)) {
    apiSendError(404, "not_found");
    return;
  }

  String rest = path.substring(prefix.length());
  int slash = rest.indexOf('/');
  String roomId = (slash < 0) ? rest : rest.substring(0, slash);
  String action = (slash < 0) ? "" : rest.substring(slash + 1);
  int room = apiRoomIndexFromId(roomId);
  if (room < 0) {
    apiSendError(404, "room_not_found");
    return;
  }

  if (action.length() == 0) {
    apiSendRoomData(room);
    return;
  }

  if (room != ROOM_SYPIALNIA) {
    apiSendError(400, "unsupported_room_control");
    return;
  }

  int switchIdx = -1;
  bool toggle = false;
  if (action == "light1") switchIdx = 0;
  else if (action == "light2") switchIdx = 1;
  else if (action == "light1/toggle") { switchIdx = 0; toggle = true; }
  else if (action == "light2/toggle") { switchIdx = 1; toggle = true; }
  else {
    apiSendError(404, "not_found");
    return;
  }

  bool target = false;
  if (toggle) {
    target = !g_tuyaSwitchStates[switchIdx];
  } else {
    DynamicJsonDocument body(128);
    if (!apiParseBody(body)) {
      apiSendError(400, "invalid_json");
      return;
    }
    if (!body["value"].is<bool>()) {
      apiSendError(400, "missing_value");
      return;
    }
    target = body["value"].as<bool>();
  }

  if (!apiSetBedroomSwitch(switchIdx, target)) {
    apiSendError(500, "tuya_command_failed");
    return;
  }

  DynamicJsonDocument doc(192);
  doc["ok"] = true;
  JsonObject data = doc.createNestedObject("data");
  data["id"] = roomId;
  data[switchIdx == 0 ? "light1" : "light2"] = g_tuyaSwitchStates[switchIdx];
  apiSendJsonDocument(200, doc);
}

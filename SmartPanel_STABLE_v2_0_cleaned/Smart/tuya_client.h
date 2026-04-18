#pragma once
#include "objects.h"
#include "state.h"
#include "room_config.h"
#include "room_logic.h"

inline String tuyaToHexUpper(const uint8_t* data, size_t len) {
  static const char hex[] = "0123456789ABCDEF";
  String out;
  out.reserve(len * 2);
  for (size_t i = 0; i < len; i++) {
    out += hex[(data[i] >> 4) & 0x0F];
    out += hex[data[i] & 0x0F];
  }
  return out;
}

inline String tuyaSha256HexLower(const String& input) {
  uint8_t hash[32];
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts(&ctx, 0);
  mbedtls_sha256_update(&ctx, (const unsigned char*)input.c_str(), input.length());
  mbedtls_sha256_finish(&ctx, hash);
  mbedtls_sha256_free(&ctx);

  const char* hex = "0123456789abcdef";
  String out;
  out.reserve(64);
  for (int i = 0; i < 32; i++) {
    out += hex[(hash[i] >> 4) & 0x0F];
    out += hex[hash[i] & 0x0F];
  }
  return out;
}

inline String tuyaHmacSha256Upper(const String& key, const String& message) {
  unsigned char hmac[32];
  mbedtls_md_context_t ctx;
  const mbedtls_md_info_t* md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);

  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, md_info, 1);
  mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
  mbedtls_md_hmac_update(&ctx, (const unsigned char*)message.c_str(), message.length());
  mbedtls_md_hmac_finish(&ctx, hmac);
  mbedtls_md_free(&ctx);

  return tuyaToHexUpper(hmac, sizeof(hmac));
}

inline String tuyaMakeNonce() {
  uint32_t a = esp_random();
  uint32_t b = esp_random();
  char buf[17];
  snprintf(buf, sizeof(buf), "%08lx%08lx", (unsigned long)a, (unsigned long)b);
  return String(buf);
}

inline String tuyaGetTimestampMs() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  uint64_t ms = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
  return String(ms);
}

inline String tuyaBuildStringToSign(const String& method, const String& body, const String& pathWithQuery) {
  String bodyHash = tuyaSha256HexLower(body);
  return method + "\n" + bodyHash + "\n\n" + pathWithQuery;
}

inline String tuyaBuildSignForToken(const String& method, const String& body, const String& pathWithQuery,
                                    const String& t, const String& nonce) {
  String stringToSign = tuyaBuildStringToSign(method, body, pathWithQuery);
  String raw = String(TUYA_CLIENT_ID) + t + nonce + stringToSign;
  return tuyaHmacSha256Upper(String(TUYA_SECRET), raw);
}

inline String tuyaBuildSignForBusiness(const String& method, const String& body, const String& pathWithQuery,
                                       const String& accessToken, const String& t, const String& nonce) {
  String stringToSign = tuyaBuildStringToSign(method, body, pathWithQuery);
  String raw = String(TUYA_CLIENT_ID) + accessToken + t + nonce + stringToSign;
  return tuyaHmacSha256Upper(String(TUYA_SECRET), raw);
}

static String   g_tuyaAccessToken   = "";
static uint32_t g_tuyaTokenExpireMs = 0;

inline bool tuyaTokenValid() {
  return g_tuyaAccessToken.length() > 0 && millis() < g_tuyaTokenExpireMs;
}

inline bool tuyaFetchToken() {
  if (!g_wifiOk) return false;
  if (time(nullptr) < 1700000000) return false;

  HTTPClient http;
  const String path  = "/v1.0/token?grant_type=1";
  const String url   = String(TUYA_BASE_URL) + path;
  const String body  = "";
  const String t     = tuyaGetTimestampMs();
  const String nonce = tuyaMakeNonce();
  const String sign  = tuyaBuildSignForToken("GET", body, path, t, nonce);

  if (!http.begin(url)) return false;

  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("sign", sign);
  http.addHeader("t", t);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("nonce", nonce);
  http.addHeader("lang", "en");

  int code = http.GET();
  String resp = http.getString();
  http.end();

  if (code <= 0) return false;

  JsonDocument doc;
  if (deserializeJson(doc, resp)) return false;
  if (!doc["success"].as<bool>()) return false;

  g_tuyaAccessToken = doc["result"]["access_token"].as<String>();
  uint32_t expireSec = doc["result"]["expire_time"] | 0;
  g_tuyaTokenExpireMs = millis() + ((expireSec > 120) ? (expireSec - 120) * 1000UL : expireSec * 1000UL);

  return g_tuyaAccessToken.length() > 0;
}

inline bool tuyaEnsureToken() {
  if (tuyaTokenValid()) return true;
  return tuyaFetchToken();
}

inline bool tuyaSendBoolCommandRaw(const String& codeName, bool value) {
  if (!tuyaEnsureToken()) return false;

  HTTPClient http;

  const String path = "/v1.0/iot-03/devices/" + String(TUYA_DEVICE_ID) + "/commands";
  const String url  = String(TUYA_BASE_URL) + path;

  JsonDocument bodyDoc;
  JsonArray commands = bodyDoc["commands"].to<JsonArray>();
  JsonObject cmd = commands.add<JsonObject>();
  cmd["code"] = codeName;
  cmd["value"] = value;

  String body;
  serializeJson(bodyDoc, body);

  const String t     = tuyaGetTimestampMs();
  const String nonce = tuyaMakeNonce();
  const String sign  = tuyaBuildSignForBusiness("POST", body, path, g_tuyaAccessToken, t, nonce);

  if (!http.begin(url)) return false;

  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("access_token", g_tuyaAccessToken);
  http.addHeader("sign", sign);
  http.addHeader("t", t);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("nonce", nonce);
  http.addHeader("lang", "en");
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(body);
  String resp = http.getString();
  http.end();

  if (code <= 0) return false;

  JsonDocument doc;
  if (deserializeJson(doc, resp)) return false;

  bool success = doc["success"] | false;
  bool result  = doc["result"]  | false;
  return success && result;
}

inline bool tuyaRefreshDeviceStates() {
  if (!tuyaEnsureToken()) return false;

  HTTPClient http;

  const String path = "/v1.0/iot-03/devices/" + String(TUYA_DEVICE_ID) + "/status";
  const String url  = String(TUYA_BASE_URL) + path;
  const String body = "";

  const String t     = tuyaGetTimestampMs();
  const String nonce = tuyaMakeNonce();
  const String sign  = tuyaBuildSignForBusiness("GET", body, path, g_tuyaAccessToken, t, nonce);

  if (!http.begin(url)) return false;

  http.addHeader("client_id", TUYA_CLIENT_ID);
  http.addHeader("access_token", g_tuyaAccessToken);
  http.addHeader("sign", sign);
  http.addHeader("t", t);
  http.addHeader("sign_method", "HMAC-SHA256");
  http.addHeader("nonce", nonce);
  http.addHeader("lang", "en");

  int code = http.GET();
  String resp = http.getString();
  http.end();

  if (code <= 0) return false;

  JsonDocument doc;
  if (deserializeJson(doc, resp)) return false;
  if (!doc["success"].as<bool>()) return false;

  JsonArray arr = doc["result"].as<JsonArray>();
  for (JsonVariant v : arr) {
    String codeName = v["code"].as<String>();
    bool value = v["value"] | false;

    if (codeName == BEDROOM_TUYA_SWITCH_CODES[0]) {
      g_tuyaSwitchStates[0] = value;
    } else if (codeName == BEDROOM_TUYA_SWITCH_CODES[1]) {
      g_tuyaSwitchStates[1] = value;
    }
  }

  return true;
}

inline void initTuya() {
  g_tuyaOk = false;
  g_tuyaUiState = TUYA_UI_DISCONNECTED;

  if (!g_wifiOk) return;
  if (time(nullptr) < 1700000000) return;

  g_tuyaUiState = TUYA_UI_CONNECTING;
  g_tuyaOk = tuyaFetchToken();

  if (g_tuyaOk) {
    g_tuyaOk = tuyaRefreshDeviceStates();
    g_tuyaUiState = g_tuyaOk ? TUYA_UI_CONNECTED : TUYA_UI_DISCONNECTED;
  } else {
    g_tuyaUiState = TUYA_UI_DISCONNECTED;
  }
}

inline void handleTuya() {
  static uint32_t lastRetryMs = 0;
  static uint32_t lastStateRefreshMs = 0;

  if (!g_wifiOk) {
    g_tuyaOk = false;
    g_tuyaUiState = TUYA_UI_DISCONNECTED;
    g_tuyaAccessToken = "";
    return;
  }

  if (!g_tuyaOk) {
    if (millis() - lastRetryMs >= TUYA_RETRY_MS) {
      lastRetryMs = millis();
      g_tuyaUiState = TUYA_UI_CONNECTING;
      g_tuyaOk = tuyaFetchToken();

      if (g_tuyaOk) {
        g_tuyaOk = tuyaRefreshDeviceStates();
        lastStateRefreshMs = millis();
        g_tuyaUiState = g_tuyaOk ? TUYA_UI_CONNECTED : TUYA_UI_DISCONNECTED;
      } else {
        g_tuyaUiState = TUYA_UI_DISCONNECTED;
      }
    }
    return;
  }

  if (!tuyaTokenValid()) {
    g_tuyaUiState = TUYA_UI_CONNECTING;
    g_tuyaOk = tuyaFetchToken();
    if (!g_tuyaOk) {
      g_tuyaUiState = TUYA_UI_DISCONNECTED;
      return;
    }
    g_tuyaUiState = TUYA_UI_CONNECTED;
  }

  if (millis() - lastStateRefreshMs >= TUYA_STATUS_REFRESH_MS) {
    lastStateRefreshMs = millis();
    g_tuyaUiState = TUYA_UI_CONNECTING;
    g_tuyaOk = tuyaRefreshDeviceStates();
    g_tuyaUiState = g_tuyaOk ? TUYA_UI_CONNECTED : TUYA_UI_DISCONNECTED;
  }
}

inline void sendRoomControl() {
  int room = roomIndexFromPage(g_currentPage);
  if (room != ROOM_SYPIALNIA) return;

  const int idx = g_selectedControlTile;
  if (idx < 0 || idx > 1) return;

  const bool targetState = !g_tuyaSwitchStates[idx];

  g_tuyaUiState = TUYA_UI_CONNECTING;
  const bool ok = tuyaSendBoolCommandRaw(BEDROOM_TUYA_SWITCH_CODES[idx], targetState);
  g_tuyaOk = ok;
  g_tuyaUiState = ok ? TUYA_UI_CONNECTED : TUYA_UI_DISCONNECTED;

  if (ok) {
    g_tuyaSwitchStates[idx] = targetState;
      g_forceFullDynamic = true;
  }
}
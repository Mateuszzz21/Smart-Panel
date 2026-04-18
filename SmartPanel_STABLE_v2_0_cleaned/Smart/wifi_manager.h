#pragma once
#include "objects.h"
#include "state.h"

inline void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  g_wifiUiState = WIFI_UI_CONNECTING;

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_CONNECT_TIMEOUT_MS) {
    delay(300);
  }

  g_wifiOk = (WiFi.status() == WL_CONNECTED);
  g_wifiUiState = g_wifiOk ? WIFI_UI_CONNECTED : WIFI_UI_DISCONNECTED;
}

inline void handleWiFi() {
  static uint32_t lastRetry = 0;

  wl_status_t st = WiFi.status();
  g_wifiOk = (st == WL_CONNECTED);

  if (g_wifiOk) {
    g_wifiUiState = WIFI_UI_CONNECTED;
    return;
  }

  if (millis() - lastRetry >= WIFI_RETRY_MS) {
    lastRetry = millis();
    WiFi.disconnect();
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    g_wifiUiState = WIFI_UI_CONNECTING;
  } else {
    if (g_wifiUiState != WIFI_UI_CONNECTING) {
      g_wifiUiState = WIFI_UI_DISCONNECTED;
    }
  }
}
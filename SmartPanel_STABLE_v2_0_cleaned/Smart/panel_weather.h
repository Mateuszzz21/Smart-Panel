#pragma once
#include "config.h"
#include "state.h"

inline int hourIndexForOffset(const JsonArray& times, int offsetHours) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return -1;

  time_t nowEpoch = mktime(&timeinfo);
  nowEpoch += offsetHours * 3600;

  struct tm targetTm;
  localtime_r(&nowEpoch, &targetTm);

  char target[20];
  snprintf(target, sizeof(target), "%04d-%02d-%02dT%02d:00",
           targetTm.tm_year + 1900,
           targetTm.tm_mon + 1,
           targetTm.tm_mday,
           targetTm.tm_hour);

  for (int i = 0; i < times.size(); i++) {
    const char* t = times[i];
    if (String(t) == String(target)) return i;
  }
  return -1;
}

inline bool fetchInternetWeather() {
  if (!WiFi.isConnected()) return false;

  HTTPClient http;
  String url =
    String("https://api.open-meteo.com/v1/forecast?latitude=") + String(WEATHER_LAT, 4) +
    "&longitude=" + String(WEATHER_LON, 4) +
    "&current=temperature_2m,weather_code" +
    "&hourly=temperature_2m,weather_code" +
    "&timezone=auto&forecast_days=1";

  http.begin(url);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return false;
  }

  DynamicJsonDocument doc(24576);
  DeserializationError err = deserializeJson(doc, http.getString());
  http.end();
  if (err) return false;

  JsonObject current = doc["current"];
  JsonObject hourly = doc["hourly"];
  JsonArray times = hourly["time"];
  JsonArray temps = hourly["temperature_2m"];
  JsonArray codes = hourly["weather_code"];

  g_weatherNowTemp = current["temperature_2m"] | NAN;
  g_weatherNowCode = current["weather_code"] | 255;

  int idx = hourIndexForOffset(times, WEATHER_FORECAST_PLUS_HOURS);
  if (idx >= 0) {
    g_weatherPlus4hTemp = temps[idx] | NAN;
    g_weatherPlus4hCode = codes[idx] | 255;
  }

  return true;
}

inline void handleInternetWeather() {
  static uint32_t lastWeatherMs = 0;
  if (!WEATHER_ENABLED) return;
  if (millis() - lastWeatherMs < WEATHER_REFRESH_MS && lastWeatherMs != 0) return;

  if (fetchInternetWeather()) {
    lastWeatherMs = millis();
  }
}
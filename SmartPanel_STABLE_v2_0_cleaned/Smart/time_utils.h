#pragma once
#include "state.h"

inline const char* getPolishWeekdayName(int wday) {
  switch (wday) {
    case 0: return "Niedziela";
    case 1: return "Poniedzialek";
    case 2: return "Wtorek";
    case 3: return "Sroda";
    case 4: return "Czwartek";
    case 5: return "Piatek";
    case 6: return "Sobota";
    default: return "";
  }
}

inline void configLocalTime() {
  configTzTime(TZ_INFO, NTP_SERVER_1, NTP_SERVER_2);
}

inline void updateClockCache() {
  static uint32_t lastClockMs = 0;
  static uint32_t lastBlinkMs = 0;

  if (millis() - lastBlinkMs >= COLON_BLINK_MS) {
    lastBlinkMs = millis();
    g_colonVisible = !g_colonVisible;
  }

  if (millis() - lastClockMs < CLOCK_REFRESH_MS) return;
  lastClockMs = millis();

  struct tm ti;
  if (!getLocalTime(&ti, 10)) return;

  char hh[3];
  char mm[3];
  char dateBuf[16];

  strftime(hh, sizeof(hh), "%H", &ti);
  strftime(mm, sizeof(mm), "%M", &ti);
  strftime(dateBuf, sizeof(dateBuf), "%d.%m.%Y", &ti);

  g_timeHH  = String(hh);
  g_timeMM  = String(mm);
  g_dateStr = String(dateBuf);
  g_dayStr  = String(getPolishWeekdayName(ti.tm_wday));
}

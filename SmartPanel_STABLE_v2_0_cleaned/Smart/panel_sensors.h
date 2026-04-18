#pragma once

#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <math.h>

#include "config.h"
#include "state.h"
#include "objects.h"

static Adafruit_BMP280 g_panelBmp;
static bool g_panelBmpOk = false;

inline void initPanelSensors() {
  Wire.begin(PANEL_I2C_SDA_PIN, PANEL_I2C_SCL_PIN);
  delay(50);

  g_panelBmpOk = g_panelBmp.begin(PANEL_BMP280_ADDR);

  if (g_panelBmpOk) {
    g_panelBmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X1,
      Adafruit_BMP280::FILTER_X4,
      Adafruit_BMP280::STANDBY_MS_250
    );
    Serial.println("[BMP280] OK");
  } else {
    Serial.print("[BMP280] BRAK pod adresem 0x");
    Serial.println(PANEL_BMP280_ADDR, HEX);
  }
}

inline void handlePanelSensors() {
  static uint32_t lastReadMs = 0;

  if (!g_panelBmpOk) return;
  if (millis() - lastReadMs < PANEL_SENSOR_READ_MS) return;
  lastReadMs = millis();

  float t = g_panelBmp.readTemperature();
  if (isnan(t)) return;

  t += TEMP_HOME_OFFSET_C;
  if (t < -40.0f || t > 85.0f) return;

  if (fabs(t - g_tempHome) >= 0.05f) {
    g_tempHome = t;
    g_hasSavedTempHome = true;
    g_prevTempHome = -999.0f;
    saveClimateStateToNvs();
  }
}
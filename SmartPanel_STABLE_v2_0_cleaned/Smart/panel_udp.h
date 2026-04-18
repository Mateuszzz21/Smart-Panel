#pragma once
#include <math.h>
#include <string.h>
#include "objects.h"
#include "state.h"
#include "room_logic.h"
#include "room_config.h"

struct __attribute__((packed)) RoomClimatePacketV1 {
  char     magic[4];
  uint8_t  room_id;
  int16_t  temp_x10;
  int16_t  hum_x10;
  uint32_t seq;
};

struct __attribute__((packed)) OutdoorTempPacketV1 {
  char     magic[4];
  int16_t  temp_x10;
  uint16_t batt_mv;
  uint8_t  batt_pct;
  uint8_t  flags;
  uint32_t seq;
};

struct __attribute__((packed)) RoomClimateBatteryPacketV1 {
  char     magic[4];
  uint8_t  room_id;
  int16_t  temp_x10;
  uint16_t batt_mv;
  uint8_t  batt_pct;
  uint8_t  flags;
  uint32_t seq;
};

inline void initPanelUdp() {
  udpDiscovery.begin(PANEL_DISCOVERY_PORT);
  udpRoomData.begin(PANEL_ROOM_DATA_PORT);
}

inline void sendPanelDiscovery() {
  static uint32_t lastSendMs = 0;
  if (!g_wifiOk) return;
  if (millis() - lastSendMs < PANEL_DISCOVERY_SEND_MS) return;
  lastSendMs = millis();

  udpDiscovery.beginPacket(IPAddress(255, 255, 255, 255), PANEL_DISCOVERY_PORT);
  udpDiscovery.print("SMARTPANEL;ROOM_PORT=");
  udpDiscovery.print(PANEL_ROOM_DATA_PORT);
  udpDiscovery.endPacket();
}

inline bool parseRoomClimateText(const String& s, uint8_t& roomId, float& tempC, float& humPct) {
  const int iRoom = s.indexOf("ROOM=");
  const int iTemp = s.indexOf("TEMP=");
  const int iHum  = s.indexOf("HUM=");
  if (iRoom < 0 || iTemp < 0 || iHum < 0) return false;

  const int endRoom = s.indexOf(';', iRoom);
  const int endTemp = s.indexOf(';', iTemp);
  const String roomStr = s.substring(iRoom + 5, endRoom < 0 ? s.length() : endRoom);
  const String tempStr = s.substring(iTemp + 5, endTemp < 0 ? s.length() : endTemp);
  const String humStr  = s.substring(iHum + 4);

  roomId = (uint8_t)roomStr.toInt();
  tempC  = tempStr.toInt() / 10.0f;
  humPct = humStr.toInt() / 10.0f;
  return true;
}

inline void applyRoomClimate(uint8_t roomId, float tempC, float humPct) {
  if (roomId >= ROOM_COUNT) return;

  g_roomTemps[roomId] = tempC;
  if (!isnan(humPct)) g_roomHums[roomId] = humPct;
  g_hasSavedRoomClimate[roomId] = true;

  int activeRoom = roomIndexFromPage(g_currentPage);
  if (activeRoom == roomId) {
    syncCurrentRoomDataForActivePage();
    g_prevRoomTemp = -999.0f;
    g_prevRoomHum  = -999.0f;
  }
}

inline void applyRoomBattery(uint8_t roomId, uint16_t battMv, uint8_t battPct, uint8_t flags, uint32_t seq) {
  if (roomId >= ROOM_COUNT) return;

  g_roomBattMv[roomId] = battMv;
  g_roomBattPct[roomId] = battPct;
  g_roomFlags[roomId] = flags;
  g_roomLastSeq[roomId] = seq;

  if (roomId == ROOM_GABINET) {
    g_prevHomeGabinetLowBat = !((flags & 0x01) != 0);
  }
  if (roomId == ROOM_SALON) {
    g_prevHomeSalonLowBat = !((flags & 0x01) != 0);
  }
}

inline void applyOutdoorTemp(float tempC) {
  g_tempOut = tempC;
  g_hasSavedTempOut = true;

  if (g_currentPage == PAGE_HOME) {
    g_prevTempOut = -999.0f;
  }
}

inline void applyOutdoorBattery(uint16_t battMv, uint8_t battPct, uint8_t flags) {
  g_outBattMv  = battMv;
  g_outBattPct = battPct;
  g_outLowBat  = (flags & 0x01) != 0;
  g_outFlags   = flags;
  g_prevHomeOutLowBat = !g_outLowBat;
}

inline void sendAckToSender(IPAddress ip, uint16_t port, uint32_t seq) {
  WiFiUDP ackUdp;
  if (!ackUdp.begin(0)) return;

  char msg[32];
  snprintf(msg, sizeof(msg), "ACK;RCB1;%lu", (unsigned long)seq);

  ackUdp.beginPacket(ip, port);
  ackUdp.write((const uint8_t*)msg, strlen(msg));
  ackUdp.endPacket();
  ackUdp.stop();

  Serial.print("[ACK] Wyslano do ");
  Serial.print(ip);
  Serial.print(":");
  Serial.print(port);
  Serial.print("  seq=");
  Serial.println(seq);
}

inline void handleRoomSensorsUdp() {
  const int packetSize = udpRoomData.parsePacket();
  if (packetSize <= 0) return;

  if (packetSize == (int)sizeof(RoomClimateBatteryPacketV1)) {
    RoomClimateBatteryPacketV1 p{};
    const int n = udpRoomData.read((char*)&p, sizeof(p));
    if (n == (int)sizeof(p) && memcmp(p.magic, "RCB1", 4) == 0) {
      g_lastUdpPacketMs = millis();
      applyRoomClimate(p.room_id, p.temp_x10 / 10.0f, NAN);
      applyRoomBattery(p.room_id, p.batt_mv, p.batt_pct, p.flags, p.seq);
      saveClimateStateToNvs();

      Serial.print("[UDP][RCB1] room=");
      Serial.print((int)p.room_id);
      Serial.print(" temp=");
      Serial.print(p.temp_x10 / 10.0f, 1);
      Serial.print(" batt=");
      Serial.print(p.batt_mv);
      Serial.print("mV ");
      Serial.print((int)p.batt_pct);
      Serial.print("% flags=");
      Serial.print((int)p.flags);
      Serial.print(" seq=");
      Serial.println((unsigned long)p.seq);

      sendAckToSender(udpRoomData.remoteIP(), 4217, p.seq);
      return;
    }
  }

  if (packetSize == (int)sizeof(RoomClimatePacketV1)) {
    RoomClimatePacketV1 p{};
    const int n = udpRoomData.read((char*)&p, sizeof(p));
    if (n == (int)sizeof(p) && memcmp(p.magic, "RCL1", 4) == 0) {
      g_lastUdpPacketMs = millis();
      applyRoomClimate(p.room_id, p.temp_x10 / 10.0f, p.hum_x10 / 10.0f);
      saveClimateStateToNvs();
      return;
    }
  }

  if (packetSize == (int)sizeof(OutdoorTempPacketV1)) {
    OutdoorTempPacketV1 p{};
    const int n = udpRoomData.read((char*)&p, sizeof(p));
    if (n == (int)sizeof(p) && memcmp(p.magic, "OUT1", 4) == 0) {
      g_lastUdpPacketMs = millis();
      applyOutdoorTemp(p.temp_x10 / 10.0f);
      applyOutdoorBattery(p.batt_mv, p.batt_pct, p.flags);
      saveClimateStateToNvs();

      Serial.print("[UDP][OUT1] temp=");
      Serial.print(p.temp_x10 / 10.0f, 1);
      Serial.print(" batt=");
      Serial.print(p.batt_mv);
      Serial.print("mV ");
      Serial.print((int)p.batt_pct);
      Serial.print("% flags=");
      Serial.print((int)p.flags);
      Serial.print(" seq=");
      Serial.println((unsigned long)p.seq);
      return;
    }
  }

  char buf[96];
  const int n = udpRoomData.read(buf, sizeof(buf) - 1);
  if (n <= 0) return;
  buf[n] = 0;

  uint8_t roomId = 255;
  float tempC = NAN;
  float humPct = NAN;
  if (parseRoomClimateText(String(buf), roomId, tempC, humPct)) {
    g_lastUdpPacketMs = millis();
    applyRoomClimate(roomId, tempC, humPct);
    saveClimateStateToNvs();
  }
}

inline void handlePanelUdp() {
  sendPanelDiscovery();
  handleRoomSensorsUdp();
}
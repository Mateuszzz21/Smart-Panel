#pragma once
#include "config.h"

enum RoomId {
  ROOM_SALON = 0,
  ROOM_GABINET,
  ROOM_SYPIALNIA,
  ROOM_COUNT
};

static const char* ROOM_NAMES[ROOM_COUNT] = {
  "SALON",
  "PRACOWNIA",
  "SYPIALNIA"
};

// Aktualnie sterowanie Tuya tylko dla Sypialni.

static const char* BEDROOM_TUYA_SWITCH_CODES[2] = {
  "switch_1",
  "switch_2"
};

#pragma once
#include "state.h"
#include "room_config.h"

inline int roomIndexFromPage(PageId p) {
  switch (p) {
    case PAGE_ROOM_SALON:     return ROOM_SALON;
    case PAGE_ROOM_GABINET:   return ROOM_GABINET;
    case PAGE_ROOM_SYPIALNIA: return ROOM_SYPIALNIA;
    default:                  return -1;
  }
}

inline String roomDisplayName(int room) {
  if (room < 0 || room >= ROOM_COUNT) return "";
  if (g_roomNames[room].length() > 0) return g_roomNames[room];
  return String(ROOM_NAMES[room]);
}

inline void syncMenuLabelsWithRoomNames() {
  for (int i = 0; i < ROOM_COUNT; i++) {
    g_menuItems[i].label = roomDisplayName(i);
  }
}

inline void loadCurrentRoomDataFromBuffers() {
  int room = roomIndexFromPage(g_currentPage);
  if (room < 0 || room >= ROOM_COUNT) return;

  g_roomTitle = roomDisplayName(room);
  g_roomTemp  = g_roomTemps[room];
  g_roomHum   = g_roomHums[room];
}

inline void syncCurrentRoomDataForActivePage() {
  int room = roomIndexFromPage(g_currentPage);
  if (room < 0 || room >= ROOM_COUNT) return;

  g_roomTitle = roomDisplayName(room);
  g_roomTemp = g_roomTemps[room];
  g_roomHum  = g_roomHums[room];
}

#pragma once

#include "ui_draw_base.h"
#include "ui_draw_icons.h"
#include "ui_draw_home.h"
#include "ui_draw_menu.h"
#include "ui_draw_room.h"
#include "ui_draw_settings.h"

inline void drawCurrentPageStatic() {
  switch (g_currentPage) {
    case PAGE_HOME:
      drawHomeStatic();
      break;

    case PAGE_MENU:
      drawMenuStatic();
      break;

    case PAGE_ROOM_SALON:
    case PAGE_ROOM_GABINET:
    case PAGE_ROOM_SYPIALNIA:
      drawRoomStatic();
      break;

    case PAGE_SETTINGS:
      drawSettingsPage();
      break;

    case PAGE_SETTINGS_INFO:
      drawInfoPage();
      break;

    case PAGE_SETTINGS_SCREEN:
      drawScreenSettingsPage();
      break;

    case PAGE_SETTINGS_ROOMS:
      drawRoomNamesPage();
      break;

    case PAGE_SETTINGS_ICONS_MENU:
      drawIconsPage();
      break;

    case PAGE_SETTINGS_ICONS_MAIN:
      drawIconsMainPage();
      break;

    case PAGE_SETTINGS_ICONS_DAY:
      drawIconsDayPage();
      break;

    case PAGE_SETTINGS_ICONS_NIGHT:
      drawIconsNightPage();
      break;

    case PAGE_ROOM_NAME_EDIT:
      drawRoomNameEditPage();
      break;
  }

  g_prevPage = g_currentPage;
}

inline void drawCurrentPageDynamic() {
  if (g_forceFullDynamic || g_currentPage != g_prevPage) {
    drawCurrentPageStatic();
    return;
  }

  switch (g_currentPage) {
    case PAGE_HOME:
      drawHomeDynamic();
      break;

    case PAGE_MENU:
      drawMenuDynamic();
      break;

    case PAGE_ROOM_SALON:
    case PAGE_ROOM_GABINET:
    case PAGE_ROOM_SYPIALNIA:
      drawRoomDynamic();
      break;

    case PAGE_SETTINGS:
      drawSettingsDynamic();
      break;

    case PAGE_SETTINGS_INFO:
      drawInfoDynamic();
      break;

    case PAGE_SETTINGS_SCREEN:
      drawScreenSettingsDynamic();
      break;

    case PAGE_SETTINGS_ROOMS:
      drawRoomNamesDynamic();
      break;

    case PAGE_SETTINGS_ICONS_MENU:
      drawIconsDynamic();
      break;

    case PAGE_SETTINGS_ICONS_MAIN:
      break;

    case PAGE_SETTINGS_ICONS_DAY:
      break;

    case PAGE_SETTINGS_ICONS_NIGHT:
      break;

    case PAGE_ROOM_NAME_EDIT:
      drawRoomNameEditDynamic();
      break;
  }
}
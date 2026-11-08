#ifndef CODUO_PRODUCT_FLAVOR_H
#define CODUO_PRODUCT_FLAVOR_H

/*
 * NOT_FROM_ORIGINAL_SOURCE: shared product-flavor selection for the
 * single-player (CoDUOSP) and multiplayer (CoDUOMP) builds.
 *
 * The retail product ships two executables that share one engine and one
 * data set but select different module, configuration, map, and script
 * names. The multiplayer spellings are the recovered defaults; the
 * single-player spellings follow the retail single-player conventions
 * (uo_gamex86.dll-style module stems, uoconfig.cfg/default.cfg, maps/<name>
 * instead of maps/mp/<name>, and the ui/ menu root). Compiling with
 * -DCODUO_SP selects the single-player flavor for every component of one
 * build; without it every spelling stays the recovered multiplayer one.
 */

#if defined(CODUO_SP)
#define CODUO_PRODUCT_IS_SP 1
#else
#define CODUO_PRODUCT_IS_SP 0
#endif

#if CODUO_PRODUCT_IS_SP

/* Retail single-player configuration files. */
#define CODUO_PRODUCT_DEFAULT_CONFIG_NAME "default.cfg"
#define CODUO_PRODUCT_AUTO_CONFIG_NAME "uoconfig.cfg"
#define CODUO_PRODUCT_AUTOEXEC_CONFIG_NAME "autoexec.cfg"
#define CODUO_PRODUCT_SAFEMODE_CONFIG_NAME "safemode.cfg"

/* Retail single-player content layout: campaign maps and their level
 * scripts live directly under maps/, with no gametype script layer. */
#define CODUO_PRODUCT_MAP_BSP_PATH_FORMAT "maps/%s.bsp"
#define CODUO_PRODUCT_LEVEL_SCRIPT_PATH "maps/%s"

/* Single-player menu set (ui/menus.txt and friends), with the multiplayer
 * set as the documented fallback for unified menu data. */
#define CODUO_PRODUCT_UI_MENU_ROOT "ui"

/* Retail single-player module identity (uo_gamex86.dll, uo_cgamex86.dll,
 * uo_uix86.dll). The architecture suffix follows the same stem. */
#define CODUO_PRODUCT_MODULE_IDENTITY_FORMAT "uo_%sx86.dll"
#define CODUO_PRODUCT_MODULE_NATIVE_ARM64_FORMAT "uo_%sarm64.dylib"
#define CODUO_PRODUCT_MODULE_NATIVE_X86_FORMAT "uo_%sx86.so"
#define CODUO_PRODUCT_MODULE_NATIVE_X86_64_FORMAT "uo_%sx86_64.so"

#define CODUO_PRODUCT_WINDOW_TITLE \
    "Call of Duty: United Offensive Single Player"

#else

/* Recovered multiplayer configuration files. */
#define CODUO_PRODUCT_DEFAULT_CONFIG_NAME "default_mp.cfg"
#define CODUO_PRODUCT_AUTO_CONFIG_NAME "uoconfig_mp.cfg"
#define CODUO_PRODUCT_AUTOEXEC_CONFIG_NAME "autoexec_mp.cfg"
#define CODUO_PRODUCT_SAFEMODE_CONFIG_NAME "safemode_mp.cfg"

/* Recovered multiplayer content layout: maps and gametype scripts under
 * maps/mp/. */
#define CODUO_PRODUCT_MAP_BSP_PATH_FORMAT "maps/mp/%s.bsp"
#define CODUO_PRODUCT_LEVEL_SCRIPT_PATH "maps/mp/%s"

#define CODUO_PRODUCT_UI_MENU_ROOT "ui_mp"

/* Recovered multiplayer module identity (uo_game_mp_x86.dll, ...). */
#define CODUO_PRODUCT_MODULE_IDENTITY_FORMAT "uo_%s_mp_x86.dll"
#define CODUO_PRODUCT_MODULE_NATIVE_ARM64_FORMAT "uo_%s_mp_arm64.dylib"
#define CODUO_PRODUCT_MODULE_NATIVE_X86_FORMAT "uo_%s_mp_x86.so"
#define CODUO_PRODUCT_MODULE_NATIVE_X86_64_FORMAT "uo_%s_mp_x86_64.so"

#define CODUO_PRODUCT_WINDOW_TITLE \
    "Call of Duty: United Offensive Multiplayer"

#endif

#endif

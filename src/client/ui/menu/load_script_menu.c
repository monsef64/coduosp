#include "../module/ui_functions.h"
#include "client/common/client_legacy_crt.h"
#include "qcommon/coduo_product_flavor.h"

#include <string.h>

#define UI_SCRIPT_MENU_DIRECTORY \
    CODUO_PRODUCT_UI_MENU_ROOT "/scriptmenus/"
#define UI_SCRIPT_MENU_FALLBACK_DIRECTORY "ui_mp/scriptmenus/"
#define UI_SCRIPT_MENU_EXTENSION ".menu"

enum {
    UI_SCRIPT_MENU_PATH_SIZE = 256,
    UI_SCRIPT_MENU_NAME_MAX =
        UI_SCRIPT_MENU_PATH_SIZE -
        (sizeof(UI_SCRIPT_MENU_DIRECTORY) - 1) -
        sizeof(UI_SCRIPT_MENU_EXTENSION)
};

// Source: uo_ui_mp_x86.dll 0x40009400..0x40009577
// Evidence: cgame_mp/mcode/uo_ui_mp_x86/FUN_40009400_40009577.mcode
// Exact same-module PPC symbol: Load_ScriptMenu.
qboolean Load_ScriptMenu(const char *menuName, int32_t loadMode)
{
    char menuPath[UI_SCRIPT_MENU_PATH_SIZE] = UI_SCRIPT_MENU_DIRECTORY;
    int32_t languageIndex;

    /* NOT_FROM_ORIGINAL_SOURCE: the directory, menu name, extension, and
     * terminator must fit the fixed path buffer. Reject an over-domain name
     * through the UI error boundary before either append. */
    if (strlen(menuName) > UI_SCRIPT_MENU_NAME_MAX) {
        trap_Error("Load_ScriptMenu: script menu name exceeds path capacity");
        return qfalse;
    }

    strcat(menuPath, menuName);
    strcat(menuPath, UI_SCRIPT_MENU_EXTENSION);

    languageIndex =
        coduo_crt_atoi(UI_Cvar_VariableString("cl_language"));
    if (languageIndex != 0) {
        char directory[UI_SCRIPT_MENU_PATH_SIZE];
        const char *basename = menuPath;
        const char *cursor;
        const char *localizedDirectory;
        const char *localizedFilename;

        Com_StripFilename(menuPath, directory);
        for (cursor = menuPath; *cursor != '\0'; ++cursor) {
            if (*cursor == '/') {
                basename = cursor + 1;
            }
        }

        localizedDirectory = va("%s%s/", directory,
                                trap_GetLanguagename(languageIndex));
        localizedFilename = va("%s%s", localizedDirectory, basename);
        if (UI_ParseMenu(localizedFilename, loadMode)) {
            return qtrue;
        }
    }

    if (UI_ParseMenu(menuPath, loadMode) != 0) {
        return qtrue;
    }
#if defined(CODUO_SP)
    /* NOT_FROM_ORIGINAL_SOURCE: fall back to the multiplayer script-menu
     * directory when the installed menu data does not provide the
     * single-player one. The fallback directory is longer than the primary
     * one, so the name bound is rechecked against its own path capacity. */
    {
        char fallbackPath[UI_SCRIPT_MENU_PATH_SIZE] =
            UI_SCRIPT_MENU_FALLBACK_DIRECTORY;
        const size_t fallbackLength =
            strlen(menuName) + (sizeof(UI_SCRIPT_MENU_FALLBACK_DIRECTORY) -
                                1u) +
            (sizeof(UI_SCRIPT_MENU_EXTENSION) - 1u);

        if (fallbackLength >= UI_SCRIPT_MENU_PATH_SIZE) {
            return qfalse;
        }
        strcat(fallbackPath, menuName);
        strcat(fallbackPath, UI_SCRIPT_MENU_EXTENSION);
        return UI_ParseMenu(fallbackPath, loadMode) != 0;
    }
#else
    return qfalse;
#endif
}

#undef UI_SCRIPT_MENU_EXTENSION
#undef UI_SCRIPT_MENU_FALLBACK_DIRECTORY
#undef UI_SCRIPT_MENU_DIRECTORY

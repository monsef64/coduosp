#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include "system_single_player_launcher.h"

#include "q_shared.h"
#include "qcommon/com_sprintf.h"

#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#endif
#endif

/*
 * NOT_FROM_ORIGINAL_SOURCE: launch the sibling single-player client. The
 * executable names follow the retail distribution layout (CoDUOSP.exe beside
 * CoDUOMP.exe; the native builds keep the same stems).
 */

#if defined(_WIN32)
#define CODUOSP_SIBLING_EXECUTABLE_NAME "CoDUOSP.exe"
#else
#define CODUOSP_SIBLING_EXECUTABLE_NAME "CoDUOSP"
#endif

#define CODUOSP_LAUNCH_FAILURE_MESSAGE \
    "Could not start " CODUOSP_SIBLING_EXECUTABLE_NAME ".\n"

static void coduosp_strip_executable_name(char *path)
{
    char *separator = NULL;

    for (char *cursor = path; *cursor != '\0'; ++cursor) {
        if (*cursor == '/' || *cursor == '\\') {
            separator = cursor;
        }
    }
    if (separator == NULL) {
        path[0] = '.';
        path[1] = '\0';
        return;
    }
    *separator = '\0';
}

/* Resolve the directory holding this executable so the sibling client is
 * launched from the same installation layout. */
static qboolean coduosp_resolve_executable_directory(char *directory,
                                                     size_t directorySize)
{
    char executablePath[MAX_OSPATH];

    if (directorySize > sizeof(executablePath)) {
        directorySize = sizeof(executablePath);
    }
#if defined(_WIN32)
    {
        const DWORD length =
            GetModuleFileNameA(NULL, executablePath,
                               (DWORD)sizeof(executablePath));

        if (length == 0 || length >= sizeof(executablePath)) {
            return qfalse;
        }
        executablePath[length] = '\0';
    }
#elif defined(__linux__)
    {
        const ssize_t length =
            readlink("/proc/self/exe", executablePath,
                     sizeof(executablePath) - 1u);

        if (length <= 0) {
            return qfalse;
        }
        executablePath[length] = '\0';
    }
#elif defined(__APPLE__)
    {
        uint32_t length = (uint32_t)sizeof(executablePath);

        if (_NSGetExecutablePath(executablePath, &length) != 0) {
            return qfalse;
        }
    }
#else
    return qfalse;
#endif
    coduosp_strip_executable_name(executablePath);
    Q_strncpyz(directory, executablePath, (int32_t)directorySize);
    return qtrue;
}

void coduosp_launch_single_player(void)
{
    char directory[MAX_OSPATH];
    char siblingPath[MAX_OSPATH];

    if (coduosp_resolve_executable_directory(directory,
                                             sizeof(directory)) == qfalse) {
        Com_Printf(CODUOSP_LAUNCH_FAILURE_MESSAGE);
        return;
    }
    Com_sprintf(siblingPath, sizeof(siblingPath), "%s/%s", directory,
                CODUOSP_SIBLING_EXECUTABLE_NAME);

#if defined(_WIN32)
    if (GetFileAttributesA(siblingPath) == INVALID_FILE_ATTRIBUTES) {
        Com_Printf(CODUOSP_LAUNCH_FAILURE_MESSAGE
                   "The file %s does not exist.\n",
                   siblingPath);
        return;
    }
    {
        STARTUPINFOA startupInfo;
        PROCESS_INFORMATION processInfo;

        memset(&startupInfo, 0, sizeof(startupInfo));
        memset(&processInfo, 0, sizeof(processInfo));
        startupInfo.cb = sizeof(startupInfo);
        if (CreateProcessA(siblingPath, NULL, NULL, NULL, FALSE, 0, NULL,
                           directory, &startupInfo,
                           &processInfo) == FALSE) {
            Com_Printf(CODUOSP_LAUNCH_FAILURE_MESSAGE);
            return;
        }
        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
    }
#else
    if (access(siblingPath, X_OK) != 0) {
        Com_Printf(CODUOSP_LAUNCH_FAILURE_MESSAGE
                   "The file %s does not exist or is not executable.\n",
                   siblingPath);
        return;
    }
    {
        const pid_t child = fork();

        if (child == 0) {
            (void)chdir(directory);
            execl(siblingPath, siblingPath, (char *)NULL);
            _exit(127);
        }
        if (child < 0) {
            Com_Printf(CODUOSP_LAUNCH_FAILURE_MESSAGE);
            return;
        }
    }
#endif
    Com_Printf("Started %s.\n", siblingPath);
}

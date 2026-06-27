#include "DebugConsole.h"

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

void InitDebugConsole()
{
#ifdef _DEBUG
    AllocConsole();

    FILE* fp;

    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONOUT$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);

    SetConsoleTitleA("Debug Log Console");

    printf("Debug console started.\n");
#endif
}

void ReleaseDebugConsole()
{
#ifdef _DEBUG
    printf("Debug console closed.\n");
    FreeConsole();
#endif
}

void DebugLog(const char* format, ...)
{
#ifdef _DEBUG
    char buffer[1024];

    va_list args;
    va_start(args, format);
    vsprintf_s(buffer, sizeof(buffer), format, args);
    va_end(args);

    printf("%s", buffer);

    OutputDebugStringA(buffer);
#endif
}
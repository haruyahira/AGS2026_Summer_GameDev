#pragma once

#include <DxLib.h>
#include <windows.h>
#include <psapi.h>

#pragma comment(lib, "psapi.lib")

struct DebugMemoryCounter
{
    int modelCount = 0;
    int modelDuplicateCount = 0;
    int animationModelCount = 0;
    int graphCount = 0;
    int soundCount = 0;
};

extern DebugMemoryCounter g_DebugMemory;

int GetPrivateMemoryMB();
int GetWorkingSetMemoryMB();

void DrawDebugMemoryInfo();
void PrintDebugMemoryInfo(const char* tag);
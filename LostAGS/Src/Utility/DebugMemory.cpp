#include "DebugMemory.h"
#include "DebugConsole.h"

DebugMemoryCounter g_DebugMemory;

int GetPrivateMemoryMB()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(
        GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
        sizeof(pmc)))
    {
        return static_cast<int>(pmc.PrivateUsage / 1024 / 1024);
    }

    return 0;
}

int GetWorkingSetMemoryMB()
{
    PROCESS_MEMORY_COUNTERS_EX pmc;

    if (GetProcessMemoryInfo(
        GetCurrentProcess(),
        reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
        sizeof(pmc)))
    {
        return static_cast<int>(pmc.WorkingSetSize / 1024 / 1024);
    }

    return 0;
}

void DrawDebugMemoryInfo()
{
#ifdef _DEBUG
    DrawFormatString(
        20,
        300,
        GetColor(255, 255, 0),
        "Private Memory : %d MB",
        GetPrivateMemoryMB()
    );

    DrawFormatString(
        20,
        320,
        GetColor(255, 180, 0),
        "WorkingSet Memory : %d MB",
        GetWorkingSetMemoryMB()
    );

    DrawFormatString(
        20,
        340,
        GetColor(100, 255, 255),
        "Model:%d Dup:%d Anim:%d Graph:%d Sound:%d",
        g_DebugMemory.modelCount,
        g_DebugMemory.modelDuplicateCount,
        g_DebugMemory.animationModelCount,
        g_DebugMemory.graphCount,
        g_DebugMemory.soundCount
    );
#endif
}

void PrintDebugMemoryInfo(const char* tag)
{
#ifdef _DEBUG
    DebugLog(
        "[MEM][%s] Private:%d MB WorkingSet:%d MB Model:%d Dup:%d Anim:%d Graph:%d Sound:%d\n",
        tag,
        GetPrivateMemoryMB(),
        GetWorkingSetMemoryMB(),
        g_DebugMemory.modelCount,
        g_DebugMemory.modelDuplicateCount,
        g_DebugMemory.animationModelCount,
        g_DebugMemory.graphCount,
        g_DebugMemory.soundCount
    );
#endif
}
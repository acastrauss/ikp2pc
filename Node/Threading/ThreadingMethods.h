#pragma once

#include <Windows.h>

/// <summary>
/// List for client connections and process them
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI ClientProcessing(LPVOID lpParam);
DWORD WINAPI WaitForSync(LPVOID lpParam);

/// <summary>
/// List for nodes connections and process them
/// </summary>
/// <param name="lpParam"></param>
/// <returns></returns>
DWORD WINAPI NodeProcessing(LPVOID lpParam);
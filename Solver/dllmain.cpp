#include <Windows.h>
#include <stdio.h>
#include <iostream>
// Get MinHook dependency
#include "Minhook.h"
#ifdef  _WIN64
#pragma comment(lib, "libMinhook.x64.lib")
#else
#pragma comment(lib, "libMinhook.x86.lib")
#endif //  _WIN64

typedef bool (__stdcall *_IsDebuggerPresent)(void);

LPVOID *og_IsDebuggerPresent = nullptr;

_IsDebuggerPresent* hk_IsDebuggerPresent() {
    return FALSE;
}


void initializeHooks() {
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
    std::cout << "Initializing Minhook" << std::endl;
    
    if (MH_Initialize() != MH_OK) {
        std::cout << "Could not initialize Minhook ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    };
    std::cout << "Initialized Minhook" << std::endl;

    // hooking IsDebuggerPresent
    if (MH_CreateHookApi(L"kernel32.dll", "IsDebuggerPresent", hk_IsDebuggerPresent, og_IsDebuggerPresent) != MH_OK) {
        std::cout << "Could not create isDebuggerPresent hook ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    }
    
    if (MH_EnableHook(GetProcAddress(GetModuleHandleA("kernel32.dll"), "IsDebuggerPresent")) != MH_OK) {
        std::cout << "Could not enable isDebuggerPresent hook ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    }

    std::cout << "Successfully hooked isDebuggerPresent" << std::endl;

}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        initializeHooks();
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


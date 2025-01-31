#include <Windows.h>
#include <stdio.h>
#include <iostream>
#include <winternl.h>

// Get MinHook dependency
#include "Minhook.h"
#ifdef  _WIN64
#pragma comment(lib, "libMinhook.x64.lib")
#else
#pragma comment(lib, "libMinhook.x86.lib")
#endif //  _WIN64

typedef bool(__stdcall *_IsDebuggerPresent)(void);
typedef bool (__stdcall *_CheckRemoteDebuggerPresent)(HANDLE, bool*);
typedef int(__stdcall *_GetWindowTextA)(HWND, LPSTR, int);

LPVOID *og_IsDebuggerPresent = nullptr;
LPVOID *og_CheckRemoteDebuggerPresent = nullptr;
LPVOID* og_GetWindowTextA = nullptr;
_IsDebuggerPresent* hk_IsDebuggerPresent() {
    return FALSE;
}

_CheckRemoteDebuggerPresent* hk_CheckRemoteDebuggerPresent(HANDLE hProcess, bool bPresent) {
    bPresent = FALSE;
    return FALSE;
}

int hk_GetWindowTextA(HWND hWindow, LPSTR str, int max) {
    int ret = strncmp("Window Title lol", str, max);
    return ret;
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

    // hooking CheckRemoteDebuggerPresent
    if (MH_CreateHookApi(L"kernel32.dll", "CheckRemoteDebuggerPresent", hk_CheckRemoteDebuggerPresent, og_CheckRemoteDebuggerPresent) != MH_OK) {
        std::cout << "Could not create CheckRemoteDebuggerPresent hook ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    }

    // hooking GetWindowTextA
    if (MH_CreateHookApi(L"user32.dll", "GetWindowTextA", hk_GetWindowTextA, og_GetWindowTextA) != MH_OK) {
        std::cout << "Could not create GetWindowTextA hook ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    }

    if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
        std::cout << "Could not enable hooks ..." << std::endl;
        printf("Error: 0x%u", GetLastError());
        ExitThread(GetCurrentThreadId());
    }




    PEB* pPEB = nullptr;

#ifdef _WIN64
    pPEB = (PEB*)__readgsqword(0x60);
#else
    pPEB = (PEB*)__readfsqword(0x30);
#endif // _WIN64

    pPEB->BeingDebugged = 0x0;

    std::cout << "Successfully hooked" << std::endl;

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


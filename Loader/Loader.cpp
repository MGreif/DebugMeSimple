#include <Windows.h>
#include <iostream>

const char HOOK_DLL_PATH[] = "Solver.dll";

typedef HMODULE(*_LoadLibraryA)(LPCSTR);

int main()
{
    std::cout << "Loading DebugMeSimple process ..." << std::endl;
    PROCESS_INFORMATION pDebugMeInfo = {};
    STARTUPINFO si = {};
    WCHAR commandLine[] = L"DebugMeSimple.exe";

    // Create suspended process so we can inject our hook dll
    if (!CreateProcessW(NULL, commandLine, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, NULL, &si, &pDebugMeInfo)) {
        std::cout << "Could not create DebugMeSimple.exe process ..." << std::endl;
        printf("Error: 0x%lu", GetLastError());
        getchar();
        ExitProcess(1);
    };
    std::cout << "Created suspended DebugMeSimple.exe process. PID: " << pDebugMeInfo.dwProcessId << std::endl;

    HANDLE hProcess = pDebugMeInfo.hProcess;


    HMODULE kernel = GetModuleHandleA("kernel32.dll");
    if (!kernel) {
        printf("Could not create kernel.dll address");
        printf("Error: 0x%lu", GetLastError());
        getchar();
        ExitProcess(1);
    }

    LPVOID pDll_path = VirtualAllocEx(hProcess, NULL, sizeof(HOOK_DLL_PATH), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    if (!pDll_path) {
        printf("Could not allocate memory");
        printf("Error: 0x%lu", GetLastError());
        getchar();
        ExitProcess(1);
    }

   

    if (!WriteProcessMemory(hProcess, pDll_path, HOOK_DLL_PATH, sizeof(HOOK_DLL_PATH), NULL)) {
        printf("Could not write memory to 0x%p\n", pDll_path);
        printf("Error: 0x%lu", GetLastError());
        getchar();
        ExitProcess(1);
    }

    _LoadLibraryA m_LoadLibraryA = (_LoadLibraryA)GetProcAddress(kernel, "LoadLibraryA");

    LPDWORD hLoaderThread = NULL;
    if (!CreateRemoteThreadEx(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)m_LoadLibraryA, pDll_path, NULL, NULL, hLoaderThread)) {
        printf("Could not create thread at address: 0x%p", m_LoadLibraryA);
        printf("Error: 0x%ul", GetLastError());
        getchar();
        ExitProcess(1);
    }
    
    std::cout << "Started thread and loaded library" << std::endl;
    MessageBox(NULL, L"Attach debugger now", L":)", MB_OK);

    ResumeThread(pDebugMeInfo.hThread);
}


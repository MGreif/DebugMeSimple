// DebugMe1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#include <thread>
#include <TlHelp32.h>
BOOL g_bIsBeingDebugged = false;
BOOL g_bParentProcessIsDebugger = false;
BOOL g_bDebuggerWindowPresent = false;


DWORD getParentProcessId() {
	DWORD currentProcessId = GetCurrentProcessId();

	DWORD processId;

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	PROCESSENTRY32W processEntry;
	if (!Process32FirstW(hSnapshot, &processEntry)) {
		return 0;
	}

	do {
		if (processEntry.th32ProcessID == currentProcessId) {
			return processEntry.th32ParentProcessID;
		}
	} while (Process32NextW(hSnapshot, &processEntry));
	return 0;
}

LONG WINAPI
VectoredHandlerBreakPoint(
	struct _EXCEPTION_POINTERS* ExceptionInfo
)
{
	if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
	{
		/*

		If a debugger is attached, this will never be executed.

		*/

		g_bIsBeingDebugged = false;
		PCONTEXT Context = ExceptionInfo->ContextRecord;

		// The breakpoint instruction is 0xCC (int 3), just one byte in size.
		// Advance to the next instruction. Otherwise, this handler will just be called ad infinitum.
	#ifdef _AMD64_
		Context->Rip++;
	#else
		Context->Eip++;
	#endif    
		// Continue execution from the instruction at Context->Rip/Eip.
		return EXCEPTION_CONTINUE_EXECUTION;
	}

	// IT's not a break intruction. Continue searching for an exception handler.
	return EXCEPTION_CONTINUE_SEARCH;
}

BOOL EnumWinProcCheckIfParentIsDebugger(HWND hWindow, LPARAM param) {
	int parentProcessId = *reinterpret_cast<int*>(param);
	
	DWORD windowProcessId;
	GetWindowThreadProcessId(hWindow, &windowProcessId);

	
	if (parentProcessId == windowProcessId) {
		std::string sTitle = std::string("", 100);
		if (strncmp(sTitle.c_str(), "", sTitle.size()) == 0) return TRUE;
		if (sTitle.find("dbg") != std::string::npos ||
			sTitle.find("Debugger") != std::string::npos ||
			sTitle.find("IDA") != std::string::npos ||
			sTitle.find("Dbg") != std::string::npos) {
			g_bParentProcessIsDebugger = true;
			return FALSE;
		}
		return FALSE;
	}
	return TRUE;
}

BOOL EnumWinProcCheckIfDebuggerPresent(HWND hWindow, LPARAM param) {

	DWORD windowProcessId;
	GetWindowThreadProcessId(hWindow, &windowProcessId);

	std::string sTitle = std::string("", 100);
	GetWindowTextA(hWindow, &sTitle[0], sTitle.size());
	if (strncmp(sTitle.c_str(), "", sTitle.size()) == 0) return TRUE;
	if (sTitle.find("dbg") != std::string::npos ||
		sTitle.find("Debugger") != std::string::npos ||
		sTitle.find("IDA") != std::string::npos ||
		sTitle.find("Dbg") != std::string::npos) {
		g_bDebuggerWindowPresent = true;
		return FALSE;
	}
	return TRUE;
}

void checkIfParentProcessIsDebugger() {
	DWORD parentProcessId = getParentProcessId();
	if (parentProcessId == 0) return;
	EnumWindows(EnumWinProcCheckIfParentIsDebugger, parentProcessId);
}

void checkIfDebuggerWindowPresent() {
	EnumWindows(EnumWinProcCheckIfDebuggerPresent, NULL);
}

void debugged() {
	MessageBox(NULL, L"I am being debugged rn >:(", L"AYO", MB_OK);
	ExitProcess(1);
}

int main()
{
	std::cout << "Hello, i will do some secret stuff here ..." << std::endl;

	// Use API to check for PEB->BeingDebugged
	// Bypass: Set PEB->BeingDebugged to 0 or hook API
	if (IsDebuggerPresent()) {
		debugged();
	}

	std::cout << "Well, you bypassed IsDebuggerPresent API :)" << std::endl;

	// Use another API to check for PEB->BeingDebugged
	// Bypass: Set PEB->BeingDebugged to 0 or hook API

	CheckRemoteDebuggerPresent(GetCurrentProcess(), &g_bIsBeingDebugged);
	if (g_bIsBeingDebugged){
		debugged();
	}


	std::cout << "Well, you bypassed CheckRemoteDebuggerPresent API :)" << std::endl;


	// Manually checking PEB->BeingDebugged flag without API
	// Bypass: Set PEB->BeingDebugged to 0
	PEB* pPEB;

#ifdef _WIN64
	pPEB = (PEB*)__readgsqword(0x60);
#else
	pPEB = (PEB*)__readfsqword(0x30);
#endif // _WIN64


	bool isBeingDebuggedFlag = pPEB->BeingDebugged;

	if (isBeingDebuggedFlag) {
		debugged();
	}


	std::cout << "Well, you bypassed direct PEB BeingDebugged flag :)" << std::endl;

	// Check if debug breakpoint exception is cought by debugger, if so the isBeingDebugged bool is not reset to false because VEH is not executed
	// Bypass: Pass exceptions back to application or patch binary
	PVOID hVeh = AddVectoredExceptionHandler(0, VectoredHandlerBreakPoint);
	g_bIsBeingDebugged = true;

	DebugBreak();

	if (hVeh)
		RemoveVectoredExceptionHandler(hVeh);

	if (g_bIsBeingDebugged) {
		debugged();
	}


	std::cout << "Well, you bypassed manual exception throw :)" << std::endl;



	checkIfParentProcessIsDebugger();

	if (g_bParentProcessIsDebugger) {
		debugged();
	}

	std::cout << "Well, you bypassed check if parent process is debugger :)" << std::endl;


	checkIfDebuggerWindowPresent();

	if (g_bDebuggerWindowPresent) {
		debugged();
	}

	std::cout << "Well, you bypassed check if debugger window is present :)" << std::endl;



	MessageBox(NULL, L"Some fancy secret stuff\nHere is your cookie: o", L"Secret", MB_OK);
	

    // Just to keep the program going
	int count = 0;
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		printf("Count: %d\n", ++count);
	}
}

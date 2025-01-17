// DebugMe1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
BOOL isBeingDebugged = false;

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

		isBeingDebugged = false;
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

	CheckRemoteDebuggerPresent(GetCurrentProcess(), &isBeingDebugged);
	if (isBeingDebugged){
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
	isBeingDebugged = true;

	DebugBreak();

	if (hVeh)
		RemoveVectoredExceptionHandler(hVeh);

	if (isBeingDebugged) {
		debugged();
	}


	std::cout << "Well, you bypassed manual exception throw :)" << std::endl;



	MessageBox(NULL, L"Some fancy secret stuff\nHere is your cookie: o", L"Secret", MB_OK);
}

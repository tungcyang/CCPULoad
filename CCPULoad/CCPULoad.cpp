// CCPULoad.cpp : Defines the entry point for the console application.
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <strsafe.h>
#include "CPUUsage.h"

#define MAX_NUM_THREADS					8		// Maximum number of threads this app might spawn.
#define MAX_CPU_USAGE_PERC				100		// Maximum allowed CPU usage percentage to be requested.
#define BUF_SIZE						255		// Size of message buffer.


HANDLE	hLoadingThreads[MAX_NUM_THREADS];
HANDLE	hWatchingThreads = INVALID_HANDLE_VALUE;
CpuUsage	usage;
ULONGLONG	ullCounter = UINT_MAX; // ULLONG_MAX;

// WatchItThreadProc() monitors the CPU usage on regular intervals.
DWORD WINAPI WatchItThreadProc(LPVOID lpParam)
{
	while (true)
	{
		fprintf(stdout, "CPU Usage: %d%%\n", usage.GetUsage());
		Sleep(APP_TICKS_INTERVAL);
	}
}

// EatItThreadProc() does nothing but a thread to consume CPU cycles.  A volatile automatic variable
// is used for this purpose.
DWORD WINAPI EatItThreadProc(LPVOID lpParam)
{
	volatile ULONGLONG	accum = 0;
	//ULONGLONG			*pullCounterLimit;
	//HANDLE	hstdOut = INVALID_HANDLE_VALUE;
	//TCHAR	msgBuf[BUF_SIZE];
	//size_t	cchStringSize;
	//DWORD	dwChars;

	//// Make sure there is a console to receive the thread output results. 
	//hstdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	//if (hstdOut == INVALID_HANDLE_VALUE)
	//	return 1;

	//// Cast the parameter to the correct data type.
	//pullCounterLimit = (ULONGLONG*)lpParam;

	//// Print the parameter values using thread-safe functions.
	//StringCchPrintf(msgBuf, BUF_SIZE, TEXT("Parameters = %ull\n"), *pullCounterLimit);
	//StringCchLength(msgBuf, BUF_SIZE, &cchStringSize);
	//WriteConsole(hstdOut, msgBuf, (DWORD)cchStringSize, &dwChars, NULL);

	while (true)
	{
		accum++;
	}

	return 0;
}

// This program is adapted from the blog article "Determine CPU usage of current process (C++ and C#)" in
// http://www.philosophicalgeek.com/2009/01/03/determine-cpu-usage-of-current-process-c-and-c/ to introduce
// a CPU load on the computer for testing purposes.
int main(int argc, char* argv[])
{
	int		i;
	int		iNumProcs, iCPUUsagePerc, iNumThreads;
	SYSTEM_INFO		sysinfo;

	// Initializations.
	for (i = 0; i < MAX_NUM_THREADS; i++)
		hLoadingThreads[i] = INVALID_HANDLE_VALUE;

	// Obtaining the number of logical processors.
	// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724958(v=vs.85).aspx.  wProcessorArchitecture can show
	// the processor architecture of the installed operating system.  For information about the physical processors
	// shared by logical processors, call GetLogicalProcessorInformationEx.
	GetSystemInfo(&sysinfo);
	iNumProcs = sysinfo.dwNumberOfProcessors;

	// Obtaining the CPU load requested, and abort if it is not valid.
	iCPUUsagePerc = atoi(argv[1]);
	if ((iCPUUsagePerc <= 0) || (iCPUUsagePerc > MAX_CPU_USAGE_PERC))
	{
		fprintf(stderr, "Please provide an integer between 0 and %d for CPU usage percentage.\n", MAX_CPU_USAGE_PERC);
		return 1;
	}

	// Start appropriate number of threads to eat the processor cycles.
	iNumThreads = (iCPUUsagePerc * iNumProcs) / 100;
	fprintf(stdout, "%d threads are needed while there are %d logical processors available.\n", iNumThreads, iNumProcs);
	// Check out "Creating Threads" in https://msdn.microsoft.com/en-us/library/windows/desktop/ms682516(v=vs.85).aspx
	for (i = 0; i < iNumThreads; i++)
	{
		hLoadingThreads[i] = CreateThread(NULL, 0, EatItThreadProc, &ullCounter, 0, NULL);

		// Check for thread creation success.  For this simple program, any failure will simply be considered
		// bad so we will exit.
		if (hLoadingThreads[i] == NULL)
		{
			fprintf(stderr, "Failure when creating the %dth thread.\n", i);
			return 1;
		}

	}

	// Start the watching thread to watch the processor (to test thread-safety)
	hWatchingThreads = CreateThread(NULL, 0, WatchItThreadProc, NULL, 0, NULL);

	while (true)
	{
		Sleep(APP_TICKS_INTERVAL);
	}

	// We are done.  Ready to clean up.
	for (i = 0; i < iNumThreads; i++)
	{
		CloseHandle(hLoadingThreads[i]);
		hLoadingThreads[i] = INVALID_HANDLE_VALUE;
	}
	CloseHandle(hWatchingThreads);
	hWatchingThreads = INVALID_HANDLE_VALUE;

    return 0;
}


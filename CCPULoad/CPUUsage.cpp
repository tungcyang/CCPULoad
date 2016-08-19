#include <windows.h>
#include "CpuUsage.h"

CpuUsage::CpuUsage(void)
: m_nCpuUsage(-1)
, m_dwLastRun(0)
, m_lRunCount(0)
{
	// Clearing the filetimes in the constructor.
	ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
	ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));

	ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
	ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
}

// CpuUsage::GetUsage() returns the percent of the CPU usage which this process has used since the last
// time this method was called.
//
// It returns -1 if there is not enough information, or called the first time.
// It returns the previous returned value if it is called "too soon" from the last call.
short CpuUsage::GetUsage()
{
	// Create a local copy to protect against race conditions in setting the member variable
	short	nCpuCopy = m_nCpuUsage;
	
	if (::InterlockedIncrement(&m_lRunCount) == 1)
	{
		// If this is called too often, the measurement itself will greatly affect the results.
		if (!EnoughTimePassed())
		{
			::InterlockedDecrement(&m_lRunCount);
			return nCpuCopy;
		}

		FILETIME	ftSysIdle, ftSysKernel, ftSysUser;
		FILETIME	ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;

		if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) ||
			!GetProcessTimes(GetCurrentProcess(), &ftProcCreation, &ftProcExit, &ftProcKernel, &ftProcUser))
		{
			::InterlockedDecrement(&m_lRunCount);
			return nCpuCopy;
		}

		if (!IsFirstRun())
		{
			// CPU usage is calculated by getting the total amount of time the process has run (kernel + user)
			// over the total amount of time the system has operated since the last measurement (kernel + user).
			ULONGLONG	ftSysKernelDiff = SubtractTimes(ftSysKernel, m_ftPrevSysKernel);
			ULONGLONG	ftSysUserDiff = SubtractTimes(ftSysUser, m_ftPrevSysUser);
			ULONGLONG	ftProcKernelDiff = SubtractTimes(ftProcKernel, m_ftPrevProcKernel);
			ULONGLONG	ftProcUserDiff = SubtractTimes(ftProcUser, m_ftPrevProcUser);
			ULONGLONG	nTotalSys = ftSysKernelDiff + ftSysUserDiff;
			ULONGLONG	nTotalProc = ftProcKernelDiff + ftProcUserDiff;

			if (nTotalSys > 0)
			{
				m_nCpuUsage = (short) ((100.0 * nTotalProc) / nTotalSys);
			}
		}

		m_ftPrevSysKernel = ftSysKernel;
		m_ftPrevSysUser = ftSysUser;
		m_ftPrevProcKernel = ftProcKernel;
		m_ftPrevProcUser = ftProcUser;

		m_dwLastRun = GetTickCount64();
		nCpuCopy = m_nCpuUsage;
	}

	::InterlockedDecrement(&m_lRunCount);
	return nCpuCopy;
}

// CpuUsage::SubtractTimes() evaluates the time difference between two file times.
ULONGLONG	CpuUsage::SubtractTimes(const FILETIME& ftA, const FILETIME& ftB)
{
	LARGE_INTEGER	a, b;
	
	a.LowPart = ftA.dwLowDateTime;
	a.HighPart = ftA.dwHighDateTime;

	b.LowPart = ftB.dwLowDateTime;
	b.HighPart = ftB.dwHighDateTime;

	return a.QuadPart - b.QuadPart;
}

// CpuUsage::EnoughTimePassed() checks if we check the timer too often.
bool	CpuUsage::EnoughTimePassed()
{
	const int minElapsedMS = MIN_CPUUSAGE_CHECK_INTERVAL;
	ULONGLONG	dwCurrentTickCount = GetTickCount64();

	return (dwCurrentTickCount - m_dwLastRun) > minElapsedMS;
}

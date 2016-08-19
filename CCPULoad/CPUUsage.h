#pragma once
#include <windows.h>

#define MIN_CPUUSAGE_CHECK_INTERVAL			250		// Periods of milliseconds that we should not check CPU usage more frequent than that.
#define APP_TICKS_INTERVAL					1000	// Number of milliseconds between two "ticks" of the app.

class CpuUsage
{
	public:
		CpuUsage(void);
		short  GetUsage();

	private:
		ULONGLONG		SubtractTimes(const FILETIME& ftA, const FILETIME& ftB);
		bool			EnoughTimePassed();
		inline bool		IsFirstRun() const { return (m_dwLastRun == 0); }

		// System total times
		FILETIME	m_ftPrevSysKernel;
		FILETIME	m_ftPrevSysUser;

		// Process times
		FILETIME	m_ftPrevProcKernel;
		FILETIME	m_ftPrevProcUser;

		short		m_nCpuUsage;
		ULONGLONG	m_dwLastRun;

		volatile LONG	m_lRunCount;
};


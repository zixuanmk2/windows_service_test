#include "stdafx.h"
#pragma comment(lib,"Userenv.lib")
#pragma comment(lib,"Wtsapi32.lib")

#include "SnailPlanService.h"
#include "ThreadPool.h"
#include "atlstr.h"
#include "Userenv.h"
#include "Wtsapi32.h"

SnailPlanService::SnailPlanService(PWSTR pszServiceName,
	BOOL fCanStop,
	BOOL fCanShutdown,
	BOOL fCanPauseContinue)
	: CServiceBase(pszServiceName, fCanStop, fCanShutdown, fCanPauseContinue)
{
	m_fStopping = FALSE;

	// Create a manual-reset event that is not signaled at first to indicate 
	// the stopped signal of the service.
	m_hStoppedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (m_hStoppedEvent == NULL)
	{
		throw GetLastError();
	}
}


SnailPlanService::~SnailPlanService()
{
	if (m_hStoppedEvent)
	{
		CloseHandle(m_hStoppedEvent);
		m_hStoppedEvent = NULL;
	}
}

//
//   FUNCTION: CSampleService::OnStart(DWORD, LPWSTR *)
//
//   PURPOSE: The function is executed when a Start command is sent to the 
//   service by the SCM or when the operating system starts (for a service 
//   that starts automatically). It specifies actions to take when the 
//   service starts. In this code sample, OnStart logs a service-start 
//   message to the Application log, and queues the main service function for 
//   execution in a thread pool worker thread.
//
//   PARAMETERS:
//   * dwArgc   - number of command line arguments
//   * lpszArgv - array of command line arguments
//
//   NOTE: A service application is designed to be long running. Therefore, 
//   it usually polls or monitors something in the system. The monitoring is 
//   set up in the OnStart method. However, OnStart does not actually do the 
//   monitoring. The OnStart method must return to the operating system after 
//   the service's operation has begun. It must not loop forever or block. To 
//   set up a simple monitoring mechanism, one general solution is to create 
//   a timer in OnStart. The timer would then raise events in your code 
//   periodically, at which time your service could do its monitoring. The 
//   other solution is to spawn a new thread to perform the main service 
//   functions, which is demonstrated in this code sample.
//
void SnailPlanService::OnStart(DWORD dwArgc, LPWSTR *lpszArgv)
{
	// Log a service start message to the Application log.
	//test
	WriteEventLogEntry(L"CppWindowsService in OnStart",
		EVENTLOG_INFORMATION_TYPE);

	HANDLE hToken = NULL;
	HANDLE hTokenNew = NULL;
	DWORD dwSessionID = WTSGetActiveConsoleSessionId();
	WTSQueryUserToken(dwSessionID, &hToken);
	if (DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityImpersonation, TokenPrimary, &hTokenNew))
	{
		STARTUPINFO startinfo = { sizeof(startinfo) };
		PROCESS_INFORMATION pi = { 0 };
		memset(&startinfo, 0, sizeof(STARTUPINFO));
		startinfo.cb = sizeof(STARTUPINFO);
		startinfo.lpDesktop = L"winsta0\\default";
		startinfo.wShowWindow = 0;

		WCHAR path[MAX_PATH] = _T("C:\\Users\\alienware`x\\Documents\\Visual Studio 2013\\Projects\\TaskTest\\Debug\\TaskTest.exe");
		PVOID env;
		CreateEnvironmentBlock(&env, hTokenNew, FALSE);
		int ierr = CreateProcessAsUser(hTokenNew,
			NULL,
			path,
			NULL,
			NULL,
			FALSE,
			CREATE_NEW_CONSOLE | NORMAL_PRIORITY_CLASS | CREATE_UNICODE_ENVIRONMENT,
			NULL,
			NULL,
			&startinfo, &pi);
		if (ierr == 0)
		{
			DWORD err = GetLastError();
			TCHAR buf[MAX_PATH];
			ZeroMemory(buf, MAX_PATH * 2);
			GetModuleFileName(NULL, buf, MAX_PATH);
			CString m_strFilename = buf;
			int nSlash = m_strFilename.ReverseFind('\\');
			m_strFilename = m_strFilename.Left(nSlash);
			m_strFilename += _T("\\config");
			CString strERROR;
			strERROR.Format(_T("%d"), err);
			WritePrivateProfileString(_T("Data"), _T("error"), strERROR, m_strFilename);
		}
	}
	// Queue the main service function for execution in a worker thread.
	CThreadPool::QueueUserWorkItem(&SnailPlanService::ServiceWorkerThread, this);
}

//
//   FUNCTION: CSampleService::ServiceWorkerThread(void)
//
//   PURPOSE: The method performs the main function of the service. It runs 
//   on a thread pool worker thread.
//
void SnailPlanService::ServiceWorkerThread(void)
{

	// Periodically check if the service is stopping.
	while (!m_fStopping)
	{

		// Perform main service function here...
		::Sleep(2000);  // Simulate some lengthy operations.
	}

	// Signal the stopped event.
	SetEvent(m_hStoppedEvent);
}


//
//   FUNCTION: CSampleService::OnStop(void)
//
//   PURPOSE: The function is executed when a Stop command is sent to the 
//   service by SCM. It specifies actions to take when a service stops 
//   running. In this code sample, OnStop logs a service-stop message to the 
//   Application log, and waits for the finish of the main service function.
//
//   COMMENTS:
//   Be sure to periodically call ReportServiceStatus() with 
//   SERVICE_STOP_PENDING if the procedure is going to take long time. 
//
void SnailPlanService::OnStop()
{
	// Log a service stop message to the Application log.
	WriteEventLogEntry(L"CppWindowsService in OnStop",
		EVENTLOG_INFORMATION_TYPE);
	// Indicate that the service is stopping and wait for the finish of the 
	// main service function (ServiceWorkerThread).
	m_fStopping = TRUE;
	if (WaitForSingleObject(m_hStoppedEvent, INFINITE) != WAIT_OBJECT_0)
	{
		throw GetLastError();
	}
}
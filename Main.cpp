#include <Windows.h>
#include <WtsApi32.h>
#include <tchar.h>
#include <cstdint>
#include <string>
#include <iostream>

#define SERVICE_NAME TEXT("AntiMalvareService")
SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
HANDLE hServiceEvent = NULL;

void WINAPI ServiceMain(DWORD argc, LPTSTR argv);
void WINAPI ServiceControlHandler(DWORD);
void ServiceInit(DWORD argc, LPTSTR argv);
void ServiceReportStatus(DWORD, DWORD, DWORD);
void CloseHandlers(SC_HANDLE, SC_HANDLE);
void ServiceDelete();
void ServiceInstall();
void ServiceStart();
void ServiceStop();
bool Check(BOOL);
void OpenManager(SC_HANDLE& hScOpenSCManager);
void Open(SC_HANDLE& hScOpenSCManager, SC_HANDLE& hScOpenService);
bool GetQueryServiceStatus(SC_HANDLE& hScOpenService, SERVICE_STATUS_PROCESS& SvcStatusProcess, DWORD& dwBytesNeeded);
int main(int argc, CHAR* argv[])
{
	std::cout << "main func Start" << std::endl;
	BOOL bStServiceCtrlDispatcher = FALSE;
	if (lstrcmpiA(argv[1], "install") == 0) 
	{
		ServiceInstall();
	}
	else if (lstrcmpiA(argv[1], "start") == 0)
	{
		ServiceStart();
	}
	else if (lstrcmpiA(argv[1], "stop") == 0)
	{
		ServiceStop();
	}
	else if (lstrcmpiA(argv[1], "delete") == 0)
	{
		ServiceDelete();
	}
	else 
	{
		SERVICE_TABLE_ENTRY DispatchTable[] =
		{
			{const_cast<LPWSTR>(SERVICE_NAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
			{NULL, NULL}
		};
		bStServiceCtrlDispatcher = StartServiceCtrlDispatcher(DispatchTable);
		if (!Check(bStServiceCtrlDispatcher)) {

		}
		else
		{

		}
	}
	
}
void WINAPI ServiceMain(DWORD argc, LPTSTR argv) 
{
	BOOL bServiceStatus = FALSE;
	hServiceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceControlHandler);
	if (hServiceStatusHandle == NULL)
	{
		std::cout << "RegisterServiceCtrlHandler failed" << GetLastError() << std::endl;
	}
	else
	{
		std::cout << "RegisterServiceCtrlHandler success" << std::endl;
	}
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (!Check(bServiceStatus))
	{
		std::cout << "Service status initial setup failed" << GetLastError() << std::endl;
	}
	else
	{
		std::cout << "Service status initial setup success" << std::endl;
	}
	ServiceInit(argc, argv);
}

void WINAPI ServiceControlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	}
}

void ServiceReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	BOOL bSetServiceStatus = FALSE;
	ServiceStatus.dwCurrentState = dwCurrentState;
	ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
	ServiceStatus.dwWaitHint = dwWaitHint;
	if (dwCurrentState == SERVICE_START_PENDING)
	{
		ServiceStatus.dwControlsAccepted = 0;
	}
	else
	{
		ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	}
	if ((dwCurrentState == SERVICE_RUNNING) || (dwCurrentState == SERVICE_STOPPED))
	{
		ServiceStatus.dwCheckPoint = 0;
	}
	else
	{
		ServiceStatus.dwCheckPoint = dwCheckPoint++;
	}
	bSetServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (!Check(bSetServiceStatus))
	{
		std::cout << "Service Status failed" << GetLastError() << std::endl;
	}
	else
	{

	}
}
void ServiceInit(DWORD argc, LPTSTR argv)
{
	hServiceEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hServiceEvent == NULL)
	{
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	else
	{
		ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
	}
	while (1)
	{
		WaitForSingleObject(hServiceEvent, INFINITE);
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
}

void ServiceDelete()
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bDeleteService = FALSE;
	Open(hScOpenSCManager, hScOpenService);
	bDeleteService = DeleteService(hScOpenService);
	if (!Check(bDeleteService))
	{
		std::cout << "Delete service failed " << " " << GetLastError() << std::endl;
	}
	else
	{
		std::cout << "Delete service success " << std::endl;
	}
	CloseHandlers(hScOpenService, hScOpenSCManager);
}

void ServiceInstall()
{
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScCreateService = NULL;
	DWORD dwGetModuleFileName = 0;
	TCHAR szPath[MAX_PATH];
	dwGetModuleFileName = GetModuleFileName(NULL, szPath, MAX_PATH);
	if (dwGetModuleFileName == 0)
	{

	}
	OpenManager(hScOpenSCManager);
	hScCreateService = CreateService(
		hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL,
		NULL,
		NULL,
		NULL,
		NULL);
	if (hScCreateService == NULL) {
		std::cout << "CreateService failed" << GetLastError() << std::endl;
		CloseServiceHandle(hScOpenSCManager);
	}
	else
	{
		std::cout << "CreateService success" << std::endl;
	}
	CloseHandlers(hScCreateService, hScOpenSCManager);
}

void ServiceStart()
{
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwBytesNeeded;
	Open(hOpenSCManager, hOpenService);
	bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);
	if (bQueryServiceStatus == FALSE) {
		std::cout << "QueryServiceStatus failed " << GetLastError() << std::endl;
	}
	else {
		std::cout << "QueryServiceStatus success" << std::endl;
	}
	if ((SvcStatusProcess.dwCurrentState != SERVICE_STOPPED) && (SvcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING))
	{
		std::cout << "Service stopped" << std::endl;
	}
	while (SvcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING)
	{
		bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);

		if (bQueryServiceStatus == FALSE)
		{
			CloseHandlers(hOpenService, hOpenSCManager);
		}
	}
	bStartService = StartService(
		hOpenService,
		NULL,
		NULL
	);
	if (bStartService == FALSE)
	{
		CloseHandlers(hOpenService, hOpenSCManager);
	}
	bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);
	if (bQueryServiceStatus == FALSE)
	{
		CloseHandlers(hOpenService, hOpenSCManager);
	}
	/*if (SvcStatusProcess.dwCurrentState != SERVICE_RUNNING)
	{
		std::cout << "Service running failed " << GetLastError() << std::endl;
		CloseHandlers(hOpenService, hOpenSCManager);
	}*/
	std::cout << "Service running success" << std::endl;
	PWTS_SESSION_INFO pSessionInfo = NULL;
	DWORD dwCount = 0;
	WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);
	LPTSTR szCmdline = const_cast<LPTSTR>(L"\"C:\\Windows\\System32\\calc.exe\"");
	for (DWORD i = 0; i < dwCount; ++i) {
		std::cout << "Session ID: " << pSessionInfo[i].SessionId << std::endl;
		if (pSessionInfo[i].SessionId != 0) {
			HANDLE hUserToken = nullptr;
			WTSQueryUserToken(pSessionInfo[i].SessionId, &hUserToken);
			STARTUPINFO si;
			PROCESS_INFORMATION pi;
			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			if (CreateProcessAsUserW(hUserToken, nullptr, szCmdline, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi)) {
				std::cout << "Calculator started for session " << pSessionInfo[i].SessionId << std::endl;
			}
			else {
				std::cerr << "Failed to start calculator for session " << pSessionInfo[i].SessionId << ": " << GetLastError() << std::endl;
			}
			CloseHandle(hUserToken);
		}
	}
	WTSFreeMemory(pSessionInfo);

	CloseHandlers(hOpenService, hOpenSCManager);
}

void ServiceStop()
{
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bQueryServiceStatus = TRUE;
	BOOL bControlService = TRUE;
	DWORD dwBytesNeeded;
	Open(hScOpenSCManager, hScOpenService);
	bQueryServiceStatus = GetQueryServiceStatus(hScOpenService, SvcStatusProcess, dwBytesNeeded);
	if (bQueryServiceStatus == FALSE)
	{
		CloseHandlers(hScOpenService, hScOpenSCManager);
	}
	bControlService = ControlService(
		hScOpenService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&SvcStatusProcess
	);
	if (bControlService == TRUE)
	{

	}
	else
	{
		CloseHandlers(hScOpenService, hScOpenSCManager);
	}
	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		bQueryServiceStatus = GetQueryServiceStatus(hScOpenService, SvcStatusProcess, dwBytesNeeded);

		if (bQueryServiceStatus == TRUE)
		{
			CloseServiceHandle(hScOpenService);
		}
		if (SvcStatusProcess.dwCurrentState == SERVICE_STOPPED)
		{
			std::cout << "Service stopped successfuly" << std::endl;
			break;
		}
		else
		{
			std::cout << "Service stopped failed" << GetLastError() << std::endl;
			CloseHandlers(hScOpenService, hScOpenSCManager);
		}
	}
	CloseHandlers(hScOpenService, hScOpenSCManager);
}

bool Check(BOOL statement)
{
	if (statement == FALSE)
	{
		return false;
	}
	return true;
}

void CloseHandlers(SC_HANDLE hScOpenService, SC_HANDLE hScOpenSCManager)
{
	CloseServiceHandle(hScOpenService);
	CloseServiceHandle(hScOpenSCManager);
}
void Open(SC_HANDLE& hScOpenSCManager, SC_HANDLE& hScOpenService) {
	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == NULL) {
		std::cout << "OpenSCManager failed" << GetLastError() << std::endl;
		return;
	}
	std::cout << "OpenSCManager success" << std::endl;
	hScOpenService = OpenService(
		hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS
	);
	if (hScOpenService == NULL)
	{
		std::cout << "OpenService failed" << GetLastError() << std::endl;
	}
	else
	{
		std::cout << "OpenService success" << std::endl;
	}
}
void OpenManager(SC_HANDLE& hScOpenSCManager)
{
	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == NULL) {
		std::cout << "OpenSCManager failed" << GetLastError() << std::endl;
		return;
	}
	std::cout << "OpenSCManager success" << std::endl;
}

bool GetQueryServiceStatus(SC_HANDLE& hScOpenService, SERVICE_STATUS_PROCESS& SvcStatusProcess, DWORD& dwBytesNeeded) {
	return QueryServiceStatusEx(
		hScOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);
}

//WTSQueryUserToken


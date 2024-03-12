#include <Windows.h>
#include <WtsApi32.h>
#include <fstream>
#include <tchar.h>
#include <cstdint>
#include <string>
#include <iostream>

#define SERVICE_NAME TEXT("AntiMalvareService")
std::fstream Log;
SERVICE_STATUS ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE hServiceStatusHandle = NULL;
HANDLE hServiceEvent = NULL;


void WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
void WINAPI ServiceControlHandler(DWORD);
void ServiceInit(DWORD argc, LPTSTR* argv);
DWORD WINAPI ServiceControlHandlerEx(DWORD dwControl, DWORD dwEventType, LPVOID dwEventData, LPVOID dwContent);
void CloseHandlers(SC_HANDLE, SC_HANDLE);
void ServiceDelete();
void ServiceInstall();
void ServiceStart();
void ServiceStop();
bool Check(BOOL);
void OpenManager(SC_HANDLE& hScOpenSCManager);
void Open(SC_HANDLE& hScOpenSCManager, SC_HANDLE& hScOpenService);
bool GetQueryServiceStatus(SC_HANDLE& hScOpenService, SERVICE_STATUS_PROCESS& SvcStatusProcess, DWORD& dwBytesNeeded);
char* GetUsernameFromSId(DWORD sId, DWORD& dwBytes);
BOOL CustomCreateProcess(DWORD wtsSession, DWORD& dwBytes);
void ServiceReportStatus(DWORD dwCurrentState, DWORD dwWin32ExitCode, DWORD dwWaitHint);
int main(int argc, CHAR* argv[])
{
	Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	if (!Log)
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in);
	Log.close();
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
	}
	
}
void WINAPI ServiceMain(DWORD argc, LPTSTR* argv) 
{
	if (Log.is_open())
		Log.sync();
	Log.close();
	Log.open("", std::ios_base::in | std::ios_base::app);
	Log << "Service Starting at " << std::endl;
	BOOL bServiceStatus = FALSE;
	hServiceStatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, (LPHANDLER_FUNCTION_EX)ServiceControlHandlerEx, NULL);
	if (hServiceStatusHandle == NULL)
	{
		Log << "RegisterServiceCtrlHandler failed" << GetLastError() << std::endl;
		return;
	}
	Log << "RegisterServiceCtrlHandler success" << std::endl;
	ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	ServiceStatus.dwControlsAccepted = SERVICE_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_SESSIONCHANGE;
	ServiceStatus.dwServiceSpecificExitCode = 0;
	ServiceReportStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	bServiceStatus = SetServiceStatus(hServiceStatusHandle, &ServiceStatus);
	if (!Check(bServiceStatus))
	{
		Log << "Service status initial setup failed" << GetLastError() << std::endl;
		return;
	}
	Log << "Service status initial setup success" << std::endl;
	PWTS_SESSION_INFO pSessionInfo = NULL;
	DWORD dwCount = 0, dwBytes = NULL;
	if (!WTSEnumerateSessionsW(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
	{
		Log << GetLastError() << std::endl;
		Log.close();
		return;
	}
	WCHAR szCmdline[] = L"C:\\Windows\\System32\\calc.exe";
	for (DWORD i = 0; i < dwCount; ++i) {
		Log << "Session ID: " << pSessionInfo[i].SessionId << std::endl;
		Log << "Session State: " << pSessionInfo[i].State << std::endl;
		if (pSessionInfo[i].SessionId != 0) {
			CustomCreateProcess(pSessionInfo[i].SessionId, dwBytes);
			/*HANDLE hUserToken;
			if (WTSQueryUserToken(pSessionInfo[i].SessionId, &hUserToken))
			{
				STARTUPINFO si{};
				ZeroMemory(&si, sizeof(STARTUPINFO));
				si.cb = sizeof(STARTUPINFO);
				si.lpDesktop = const_cast<wchar_t*>(L"WinSta0\\Default");
				PROCESS_INFORMATION pi{};
				if (CreateProcessAsUserW(hUserToken, NULL, szCmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
					std::cout << "Calculator started for session " << pSessionInfo[i].SessionId << std::endl;
				}
				else {
					std::cerr << "Failed to start calculator for session " << pSessionInfo[i].SessionId << ": " << GetLastError() << std::endl;
				}
				CloseHandle(hUserToken);
			}
			else
			{
				std::cout << "hUserToken " << hUserToken << std::endl;
				std::cerr << "Failed to get user token for session " << pSessionInfo[i].SessionId << ": " << GetLastError() << std::endl;
			}*/
		}
	}
	WTSFreeMemory(pSessionInfo);
	ServiceInit(argc, argv);
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	Log << "Service Initialization complete at " << std::endl
		<< "Service begins to performing" << std::endl;
	while (ServiceStatus.dwCurrentState != SERVICE_STOPPED) {
		Log << "Service is running. There are no problems on your PC. " << std::endl;
		Log.sync();
		if (WaitForSingleObject(hServiceEvent, 60000) != WAIT_TIMEOUT)
			ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}
	Log.close();
}

DWORD WINAPI ServiceControlHandlerEx(DWORD CtrlCode, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContent)
{
	DWORD errorCode = NO_ERROR, dwBytes = NULL;
	WTSSESSION_NOTIFICATION* sessionNotification = NULL;
	char* pcUserName = nullptr;
	if (Log.is_open())
		Log.sync();
	Log.close();
	Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_INTERROGATE:
		break;
	case SERVICE_CONTROL_SESSIONCHANGE:
		sessionNotification = static_cast<WTSSESSION_NOTIFICATION*>(lpEventData);
		pcUserName = GetUsernameFromSId(sessionNotification->dwSessionId, dwBytes);
		if (dwEventType == WTS_SESSION_LOGOFF) {
			Log << pcUserName << " is logging off" << std::endl;
			break;
		}
		else if (dwEventType == WTS_SESSION_LOGON) {
			Log << pcUserName << " is logging in" << std::endl;
			CustomCreateProcess(sessionNotification->dwSessionId, dwBytes);
		}
		delete[] pcUserName;
		break;
	case SERVICE_CONTROL_STOP:
		Log << "Service have been Stopped at " << std::endl;
		ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		//ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	case SERVICE_CONTROL_SHUTDOWN:
		Log << "PC is going to SHUTDOWN stopping the Service" << std::endl;
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		break;
	default:
		errorCode = ERROR_CALL_NOT_IMPLEMENTED;
	}
	Log.close();
	ServiceReportStatus(ServiceStatus.dwCurrentState, errorCode, 0);
	return errorCode;
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
void ServiceInit(DWORD argc, LPTSTR* argv)
{
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	hServiceEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (hServiceEvent == NULL)
	{
		ServiceReportStatus(SERVICE_STOPPED, NO_ERROR, 0);
		return;
	}
	ServiceReportStatus(SERVICE_RUNNING, NO_ERROR, 0);
}

void ServiceDelete()
{
	Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScOpenService = NULL;
	BOOL bDeleteService = FALSE;
	Open(hScOpenSCManager, hScOpenService);
	bDeleteService = DeleteService(hScOpenService);
	if (!Check(bDeleteService))
	{
		Log << "Delete service failed " << " " << GetLastError() << std::endl;
	}
	else
	{
		Log << "Delete service success " << std::endl;
	}
	CloseHandlers(hScOpenService, hScOpenSCManager);
}

void ServiceInstall()
{
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	SC_HANDLE hScOpenSCManager = NULL;
	SC_HANDLE hScCreateService = NULL;
	DWORD dwGetModuleFileName = 0;
	TCHAR szPath[MAX_PATH];
	dwGetModuleFileName = GetModuleFileName(NULL, szPath, MAX_PATH);
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
		Log << "CreateService failed" << GetLastError() << std::endl;
		CloseServiceHandle(hScOpenSCManager);
	}
	else
	{
		Log << "CreateService success" << std::endl;
	}
	CloseHandlers(hScCreateService, hScOpenSCManager);
}

void ServiceStart()
{
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	BOOL bStartService = FALSE;
	SERVICE_STATUS_PROCESS SvcStatusProcess;
	SC_HANDLE hOpenSCManager = NULL;
	SC_HANDLE hOpenService = NULL;
	BOOL bQueryServiceStatus = FALSE;
	DWORD dwBytesNeeded;
	Open(hOpenSCManager, hOpenService);
	bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);
	if (bQueryServiceStatus == FALSE) {
		Log << "QueryServiceStatus failed " << GetLastError() << std::endl;
	}
	else {
		Log << "QueryServiceStatus success" << std::endl;
	}
	if ((SvcStatusProcess.dwCurrentState != SERVICE_STOPPED) && (SvcStatusProcess.dwCurrentState != SERVICE_STOP_PENDING))
	{
		Log << "Service stopped" << std::endl;
		CloseHandlers(hOpenService, hOpenSCManager);
	}

	while (SvcStatusProcess.dwCurrentState == SERVICE_STOP_PENDING)
	{
		bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);

		if (bQueryServiceStatus == FALSE)
		{
			CloseHandlers(hOpenService, hOpenSCManager);
			return;
		}
	}
	bStartService = StartService(
		hOpenService,
		NULL,
		NULL
	);
	if (bStartService == FALSE)
	{
		Log << "StartService failed (%d)\n" << GetLastError();
		CloseHandlers(hOpenService, hOpenSCManager);
		return;
	}
	bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);
	if (bQueryServiceStatus == FALSE)
	{
		Log << "QueryServiceStatusEx failed (%d)\n" << GetLastError();
		CloseHandlers(hOpenService, hOpenSCManager);
	}
	while (SvcStatusProcess.dwCurrentState == SERVICE_START_PENDING)
	{
		bQueryServiceStatus = GetQueryServiceStatus(hOpenService, SvcStatusProcess, dwBytesNeeded);
		if (bQueryServiceStatus == FALSE)
		{
			Log << "QueryServiceStatusEx failed (%d)\n" << GetLastError();
			break;
		}
	}
	if (SvcStatusProcess.dwCurrentState != SERVICE_RUNNING)
	{
		Log << "Service running failed " << GetLastError() << std::endl;
		CloseHandlers(hOpenService, hOpenSCManager);
	}
	Log << "Service running success" << std::endl;
	CloseHandlers(hOpenService, hOpenSCManager);
}

void ServiceStop()
{
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
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
	if (bControlService == FALSE)
		CloseHandlers(hScOpenService, hScOpenSCManager);
	while (SvcStatusProcess.dwCurrentState != SERVICE_STOPPED)
	{
		bQueryServiceStatus = GetQueryServiceStatus(hScOpenService, SvcStatusProcess, dwBytesNeeded);

		if (bQueryServiceStatus == TRUE)
		{
			CloseServiceHandle(hScOpenService);
		}
		if (SvcStatusProcess.dwCurrentState == SERVICE_STOPPED)
		{
			Log << "Service stopped successfuly" << std::endl;
			break;
		}
		else
		{
			Log << "Service stopped failed" << GetLastError() << std::endl;
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
		//std::cout << "OpenSCManager failed" << GetLastError() << std::endl;
		return;
	}
	//std::cout << "OpenSCManager success" << std::endl;
	hScOpenService = OpenService(
		hScOpenSCManager,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS
	);
	if (hScOpenService == NULL)
	{
		//std::cout << "OpenService failed" << GetLastError() << std::endl;
	}
	else
	{
		//std::cout << "OpenService success" << std::endl;
	}
}
void OpenManager(SC_HANDLE& hScOpenSCManager)
{
	hScOpenSCManager = OpenSCManager(
		NULL,
		NULL,
		SC_MANAGER_ALL_ACCESS);
	if (hScOpenSCManager == NULL) {
		//std::cout << "OpenSCManager failed" << GetLastError() << std::endl;
		return;
	}
	//std::cout << "OpenSCManager success" << std::endl;
}

bool GetQueryServiceStatus(SC_HANDLE& hScOpenService, SERVICE_STATUS_PROCESS& SvcStatusProcess, DWORD& dwBytesNeeded) {
	return QueryServiceStatusEx(
		hScOpenService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&SvcStatusProcess,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded);
}

BOOL CustomCreateProcess(DWORD wtsSession, DWORD& dwBytes) {
	if (!Log.is_open())
		Log.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\Log.txt", std::ios_base::in | std::ios_base::app);
	HANDLE userToken;
	PROCESS_INFORMATION pi{};
	STARTUPINFO si{};
	WCHAR path[] = L"C:\\Windows\\System32\\cmd.exe";
	WTSQueryUserToken(wtsSession, &userToken);
	CreateProcessAsUser(userToken, NULL, path, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	char* pcUserName = GetUsernameFromSId(wtsSession, dwBytes);
	Log << "Application pId " << pi.dwProcessId << " have been started by user " << pcUserName
		<< " in session " << wtsSession << " at " << std::endl;
	delete[] pcUserName;
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}



char* GetUsernameFromSId(DWORD sId, DWORD& dwBytes) {
	char* pcUserName = new char[105];
	LPWSTR buff;
	WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE, sId, WTSUserName, &buff, &dwBytes);
	WideCharToMultiByte(CP_UTF8, 0, buff, -1, pcUserName, 105, 0, 0);
	WTSFreeMemory(buff);
	return pcUserName;
}

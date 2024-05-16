#include "Header.h"

NamedPipeClientStream::NamedPipeClientStream(const std::u16string& name, uint32_t timeout)
	: HandleStream(ConnectToServicePipe(name, timeout))
{
}

NamedPipeClientStream::~NamedPipeClientStream()
{
}

HANDLE NamedPipeClientStream::ConnectToServicePipe(const std::u16string& name, uint32_t timeout)
{
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	while (true)
	{
		hPipe = CreateFileW(
			reinterpret_cast<LPCWSTR>(name.c_str()),
			GENERIC_READ |
			GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);
		if (hPipe != INVALID_HANDLE_VALUE)
		{
			break;
		}
		if (GetLastError() != ERROR_PIPE_BUSY)
		{
			return INVALID_HANDLE_VALUE;
		}
		if (!WaitNamedPipe(reinterpret_cast<LPCWSTR>(name.c_str()), timeout))
		{
			return INVALID_HANDLE_VALUE;
		}
	}
	DWORD dwMode = PIPE_READMODE_MESSAGE;
	BOOL fSuccess = SetNamedPipeHandleState(
		hPipe,
		&dwMode,
		NULL,
		NULL);
	if (!fSuccess)
	{
		return INVALID_HANDLE_VALUE;
	}
	return hPipe;
}

NamedPipeServerStream::NamedPipeServerStream(const std::u16string& name)
	: HandleStream(
		CreateNamedPipeW(
			reinterpret_cast<LPCWSTR>(name.c_str()),
			PIPE_ACCESS_DUPLEX,
			PIPE_TYPE_MESSAGE |
			PIPE_READMODE_MESSAGE |
			PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			BUFSIZE,
			BUFSIZE,
			0,
			NULL)
	)
{
}

bool NamedPipeServerStream::WaitConnection()
{
	BOOL fConnected = ConnectNamedPipe(m_handle, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	return fConnected;
}

NamedPipeServerStream::~NamedPipeServerStream()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		FlushFileBuffers(m_handle);
		DisconnectNamedPipe(m_handle);
	}
}
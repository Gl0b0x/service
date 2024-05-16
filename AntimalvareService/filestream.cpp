#include "Header.h"

HandleStream::HandleStream(HANDLE handle) : m_handle(handle)
{
}

HandleStream::~HandleStream()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
	}
}

uint32_t HandleStream::Write(uint8_t* data, uint32_t length)
{
	DWORD cbWritten = 0;
	BOOL fSuccess = WriteFile(
		m_handle,
		data,
		length,
		&cbWritten,
		NULL
	);
	if (!fSuccess || length != cbWritten)
	{
		return false;
	}
	return cbWritten;
}

uint32_t HandleStream::Read(uint8_t* data, uint32_t length)
{
	DWORD cbBytesRead = 0;
	BOOL fSuccess = ReadFile(
		m_handle,
		data,
		length,
		&cbBytesRead,
		NULL
	);
	if (!fSuccess || length != cbBytesRead)
	{
		return false;
	}
	return cbBytesRead;
}
FileStream::FileStream(const std::u16string& name)
	: HandleStream(CreateFileW(
		reinterpret_cast<LPCWSTR>(name.c_str()),
		GENERIC_READ |
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_ALWAYS,
		0,
		NULL)
	)
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		WriteLog(std::wstring(name.begin(), name.end()) + L" opened successfully.");
	}
	else
	{
		DWORD error = GetLastError();
		WriteLog(L"Failed to open file: error code " + std::to_wstring(error));
	}
}

uint32_t FileStream::Write(uint8_t* data, uint32_t length)
{
	return HandleStream::Write(data, length);
}
uint32_t FileStream::Read(uint8_t* data, uint32_t length)
{
	return HandleStream::Read(data, length);
}
int64_t FileStream::GetCurrentPosition() const
{
	LARGE_INTEGER newPosition = { 0 };
	LARGE_INTEGER currentPosition = { 0 };
	SetFilePointerEx(m_handle, newPosition, &currentPosition, FILE_CURRENT);
	return currentPosition.QuadPart;
}
void FileStream::SetCurrentPosition(int64_t position, MoveMethod moveMethod)
{
	LARGE_INTEGER newPosition = { 0 };
	LARGE_INTEGER currentPosition = { 0 };
	SetFilePointerEx(m_handle, newPosition, &currentPosition, ConvertMoveMethod(moveMethod));
}
int64_t FileStream::GetSize() const
{
	LARGE_INTEGER li = { 0 };
	if (GetFileSizeEx(m_handle, &li) == 0)
	{
		return -1;
	}
	return li.QuadPart;
}
DWORD FileStream::ConvertMoveMethod(MoveMethod moveMethod)
{
	switch (moveMethod)
	{
	case MoveMethod::Begin:
		return FILE_BEGIN;
	case MoveMethod::Current:
		return FILE_CURRENT;
	case MoveMethod::End:
		return FILE_END;
	default:
		return FILE_BEGIN;
	}
}



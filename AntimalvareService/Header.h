#pragma once
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "WtsApi32.lib")
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <WtsApi32.h>
#include <iostream>
#include <fstream>
#include "SDDL.h"
#include <thread>
#include <chrono>
#include <string>
#include <format>
#include <map>
#include <vector>
typedef std::basic_string<TCHAR> Tstring;
#define BUFSIZE 512
extern std::wofstream errorLog;
template<typename T>
VOID WriteLog(const T& data, Tstring prefix = __TEXT(""))
{
	if (!errorLog.is_open())
		errorLog.open("C:\\Users\\79195\\Desktop\\RBPO\\AntimalvareService\\log.txt", std::ios::app);
	errorLog << prefix << data << std::endl;
}
enum class HashType
{
	HashSha1, HashMd5, HashSha256
};
enum class MoveMethod
{
	Begin,
	Current,
	End
};

struct MyIStream
{
	virtual uint32_t Write(uint8_t* data, uint32_t length) = 0;
	virtual uint32_t Read(uint8_t* data, uint32_t length) = 0;
	virtual ~MyIStream() = default;
};

struct IRandomAccessStream : MyIStream
{
	virtual int64_t GetCurrentPosition() const = 0;
	virtual void SetCurrentPosition(int64_t position, MoveMethod moveMethod) = 0;
	virtual int64_t GetSize() const = 0;
};

class HandleStream : public MyIStream
{
public:
	explicit HandleStream(HANDLE handle = INVALID_HANDLE_VALUE);
	~HandleStream();
	virtual uint32_t Write(uint8_t* data, uint32_t length) override;
	virtual uint32_t Read(uint8_t* data, uint32_t length) override;
protected:
	HANDLE m_handle;
};
class FileStream : public IRandomAccessStream, public HandleStream
{
public:
	FileStream(const std::u16string& name);
	uint32_t Write(uint8_t* data, uint32_t length) override;
	uint32_t Read(uint8_t* data, uint32_t length) override;
	int64_t GetCurrentPosition() const override;
	void SetCurrentPosition(int64_t position, MoveMethod moveMethod) override;
	int64_t GetSize() const override;
private:
	DWORD ConvertMoveMethod(MoveMethod moveMethod);
};

struct IDataWriter
{
	virtual bool WriteUInt8(uint8_t data) = 0;
	virtual bool WriteUInt16(uint16_t data) = 0;
	virtual bool WriteUInt32(uint32_t data) = 0;
	virtual bool WriteUInt64(uint64_t data) = 0;

	virtual bool WriteInt8(uint8_t data) = 0;
	virtual bool WriteInt16(uint16_t data) = 0;
	virtual bool WriteInt32(uint32_t data) = 0;
	virtual bool WriteInt64(uint64_t data) = 0;

	virtual bool WriteFloat32(float data) = 0;
	virtual bool WriteFloat64(double data) = 0;
	virtual bool WriteChar(char data) = 0;
	virtual bool WriteBool(bool data) = 0;
	virtual bool WriteString(const std::string& data) = 0;
	virtual bool WriteBlob(const std::vector<uint8_t>& data) = 0;
	virtual ~IDataWriter() = default;
};

struct IDataReader
{
	virtual bool ReadUInt8(uint8_t& data) = 0;
	virtual bool ReadUInt16(uint16_t& data) = 0;
	virtual bool ReadUInt32(uint32_t& data) = 0;
	virtual bool ReadUInt64(uint64_t& data) = 0;

	virtual bool ReadInt8(uint8_t& data) = 0;
	virtual bool ReadInt16(uint16_t& data) = 0;
	virtual bool ReadInt32(uint32_t& data) = 0;
	virtual bool ReadInt64(uint64_t& data) = 0;

	virtual bool ReadFloat32(float& data) = 0;
	virtual bool ReadFloat64(double& data) = 0;
	virtual bool ReadChar(char& data) = 0;
	virtual bool ReadBool(bool& data) = 0;
	virtual bool ReadString(std::string& data) = 0;
	virtual bool ReadBlob(std::vector<uint8_t>& data) = 0;
	virtual ~IDataReader() = default;
};

class BinarySerializer : public IDataReader, public IDataWriter
{
public:
	explicit BinarySerializer(const std::shared_ptr<MyIStream>& stream);
	virtual bool ReadUInt8(uint8_t& data) override;
	virtual bool ReadUInt16(uint16_t& data) override;
	virtual bool ReadUInt32(uint32_t& data) override;
	virtual bool ReadUInt64(uint64_t& data) override;
	virtual bool ReadInt8(uint8_t& data) override;
	virtual bool ReadInt16(uint16_t& data) override;
	virtual bool ReadInt32(uint32_t& data) override;
	virtual bool ReadInt64(uint64_t& data) override;
	virtual bool ReadFloat32(float& data) override;
	virtual bool ReadFloat64(double& data) override;
	virtual bool ReadChar(char& data) override;
	virtual bool ReadBool(bool& data) override;
	virtual bool ReadString(std::string& data) override;
	virtual bool ReadBlob(std::vector<uint8_t>& data) override;

	virtual bool WriteUInt8(uint8_t data) override;
	virtual bool WriteUInt16(uint16_t data) override;
	virtual bool WriteUInt32(uint32_t data) override;
	virtual bool WriteUInt64(uint64_t data) override;
	virtual bool WriteInt8(uint8_t data) override;
	virtual bool WriteInt16(uint16_t data) override;
	virtual bool WriteInt32(uint32_t data) override;
	virtual bool WriteInt64(uint64_t data) override;
	virtual bool WriteFloat32(float data) override;
	virtual bool WriteFloat64(double data) override;
	virtual bool WriteChar(char data) override;
	virtual bool WriteBool(bool data) override;
	virtual bool WriteString(const std::string& data) override;
	virtual bool WriteBlob(const std::vector<uint8_t>& data) override;
private:
	std::shared_ptr<MyIStream> m_stream;
};

struct Record
{
	uint64_t prefix;
	uint32_t len;
	std::vector<uint8_t> hash;
	uint32_t offsetStart;
	uint32_t offsetEnd;
	std::string name;
};
class AvBases
{
public:
	AvBases(const std::multimap<uint64_t, Record>& data);
	std::vector<Record> FindRecord(uint64_t key);
private:
	std::multimap<uint64_t, Record> records;
};
class AvBasesLoader
{
public:
	std::shared_ptr<AvBases> LoadBases(std::u16string path);
};

class NamedPipeClientStream : public HandleStream
{
public:
	NamedPipeClientStream(const std::u16string& name, uint32_t timeout);
	~NamedPipeClientStream();
private:
	HANDLE ConnectToServicePipe(const std::u16string& name, uint32_t timeout);
};

class NamedPipeServerStream : public HandleStream
{
public:
	explicit NamedPipeServerStream(const std::u16string& name);
	bool WaitConnection();
	~NamedPipeServerStream();
};
class ScanEngine
{
public:
	ScanEngine(const std::shared_ptr<AvBases>& bases);
	bool Scan(const std::shared_ptr<IRandomAccessStream> stream, std::string& malwareName);
private:
	std::shared_ptr<AvBases> bases;
};
Tstring GetUserID(HANDLE userToken);
SECURITY_ATTRIBUTES GetSecurityAttributes(const Tstring& sdd1);
BOOL Read(HANDLE handle, uint8_t* data, uint64_t lenght, DWORD& bytesRead);
BOOL Write(HANDLE handle, uint8_t* data, uint64_t lenght);
std::vector<uint8_t> GetDataHash(const uint8_t* data, const size_t data_size, HashType hashType);

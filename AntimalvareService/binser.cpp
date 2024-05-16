#include "Header.h"
template<typename T>
extern VOID WriteLog(const T& data, Tstring);

BinarySerializer::BinarySerializer(const std::shared_ptr<MyIStream>& stream)
	:m_stream(stream)
{
}

bool BinarySerializer::ReadUInt8(uint8_t& data)
{
	return m_stream->Read(&data, sizeof(data));
}
bool BinarySerializer::ReadUInt16(uint16_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadUInt32(uint32_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadUInt64(uint64_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadInt8(uint8_t& data)
{
	return m_stream->Read(&data, sizeof(data));
}
bool BinarySerializer::ReadInt16(uint16_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadInt32(uint32_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadInt64(uint64_t& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::ReadFloat32(float& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::ReadFloat64(double& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::ReadChar(char& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::ReadBool(bool& data)
{
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::ReadString(std::string& data)
{
	uint64_t size = 0;
	if (ReadUInt64(size))
	{
		data.resize(size);
		return m_stream->Read(reinterpret_cast<uint8_t*>(data.data()), size);

	}
	return m_stream->Read(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::ReadBlob(std::vector<uint8_t>& data)
{
	uint32_t bytesRead = m_stream->Read(reinterpret_cast<uint8_t*>(data.data()), data.size());
	WriteLog(bytesRead);
	if (bytesRead > 0)
	{
		if (bytesRead != data.size())
		{
			data.resize(bytesRead);
		}
		data.resize(bytesRead);
		return true;
	}
	return false;
}
bool BinarySerializer::WriteUInt8(uint8_t data)
{
	return sizeof(data) == m_stream->Write(&data, sizeof(data));
}

bool BinarySerializer::WriteUInt16(uint16_t data)
{
	return m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteUInt32(uint32_t data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteUInt64(uint64_t data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}
bool BinarySerializer::WriteInt8(uint8_t data)
{
	return sizeof(data) == m_stream->Write(&data, sizeof(data));
}

bool BinarySerializer::WriteInt16(uint16_t data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteInt32(uint32_t data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteInt64(uint64_t data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteFloat32(float data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteFloat64(double data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteChar(char data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteBool(bool data)
{
	return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(&data), sizeof(data));
}

bool BinarySerializer::WriteString(const std::string& data)
{
	if (WriteUInt64(data.size()))
	{
		return sizeof(data) == m_stream->Write(reinterpret_cast<uint8_t*>(const_cast<char*>(data.c_str())), data.size());
	}
	return false;
}

bool BinarySerializer::WriteBlob(const std::vector<uint8_t>& data)
{
	uint64_t size = data.size();
	return size == m_stream->Write(reinterpret_cast<uint8_t*>(const_cast<uint8_t*>(data.data())), size);
}
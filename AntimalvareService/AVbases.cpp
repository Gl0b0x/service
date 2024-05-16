#include "Header.h"
template<typename T>
extern VOID WriteLog(const T& data, Tstring prefix);
AvBases::AvBases(const std::multimap<uint64_t, Record>& data)
	: records{ data }
{};

std::vector<Record> AvBases::FindRecord(uint64_t key)
{
	auto range = records.equal_range(key);
	std::vector<Record> records;
	for (auto it = range.first; it != range.second; ++it)
	{
		records.push_back(it->second);
	}
	return records;
}

std::shared_ptr<AvBases> AvBasesLoader::LoadBases(std::u16string path)
{
	std::multimap<uint64_t, Record>records;
	auto file = std::make_shared<FileStream>(path);
	BinarySerializer serializer(static_pointer_cast<IRandomAccessStream>(file));
	std::vector<uint8_t> data(9);
	std::vector<uint8_t> expectSig({ 'K','u','z','n','e','t','s','o','v' });
	if (!serializer.ReadBlob(data)
		|| !(data == expectSig)
		)
	{

		return nullptr;
	}
	uint64_t count;
	if (!serializer.ReadUInt64(count))
	{
		return nullptr;
	}
	for (int i = 0; i < count; i++)
	{
		std::vector<uint8_t> hash;
		Record record;
		if (!serializer.ReadUInt64(record.prefix))
		{
			return nullptr;
		}
		if (!serializer.ReadUInt32(record.len))
		{
			return nullptr;
		}
		record.hash.resize(32);
		if (!serializer.ReadBlob(record.hash))
		{
			return nullptr;
		}
		if (!serializer.ReadUInt32(record.offsetStart))
		{
			return nullptr;
		}
		if (!serializer.ReadUInt32(record.offsetEnd))
		{
			return nullptr;
		}
		if (!serializer.ReadString(record.name))
		{
			return nullptr;
		}
		records.insert(std::make_pair(record.prefix, record));
	}
	WriteLog("файл успешно прочитан");
	return std::make_shared<AvBases>(records);
}

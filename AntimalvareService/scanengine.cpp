#include "Header.h"
ScanEngine::ScanEngine(const std::shared_ptr<AvBases>& bases)
	: bases(bases)
{ }
bool ScanEngine::Scan(const std::shared_ptr<IRandomAccessStream> stream, std::string& malwareName)
{
	if (!stream)
	{
		return false;
	}
	BinarySerializer s(stream);
	do
	{
		uint64_t position = stream->GetCurrentPosition();
		uint64_t prefix;
		if (!s.ReadUInt64(prefix))
		{
			return false;
		}
		auto records = bases->FindRecord(prefix);
		for (const auto& record : records)
		{
			if (position < record.offsetStart || position > record.offsetEnd)
			{
				continue;
			}
			stream->SetCurrentPosition(position, MoveMethod::Begin);
			std::vector<uint8_t> data(record.len);
			s.ReadBlob(data);
			auto hash = GetDataHash(data.data(), data.size(), HashType::HashSha256);
			if (hash == record.hash)
			{
				malwareName = record.name;
				return true;
			}
		}
		stream->SetCurrentPosition(1, MoveMethod::Current);
	} while (true);
}

std::vector<uint8_t> GetDataHash(const uint8_t* data, const size_t data_size, HashType hashType)
{
	HCRYPTPROV hProv = NULL;
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
	{
		return {};
	}
	BOOL hash_ok = FALSE;
	HCRYPTPROV hHash = NULL;
	switch (hashType) {
	case HashType::HashSha1: hash_ok = CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash); break;
	case HashType::HashMd5: hash_ok = CryptCreateHash(hProv, CALG_MD5, 0, 0, &hHash); break;
	case HashType::HashSha256: hash_ok = CryptCreateHash(hProv, CALG_SHA_256, 0, 0, &hHash); break;
	}
	if (!hash_ok) {
		CryptReleaseContext(hProv, 0);
		return {};
	}
	if (!CryptHashData(hHash, static_cast<const BYTE*>(data), data_size, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return {};
	}
	DWORD cbHashSize = 0, dwCount = sizeof(DWORD);
	if (!CryptGetHashParam(hHash, HP_HASHSIZE, (BYTE*)&cbHashSize, &dwCount, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return {};
	}
	std::vector<uint8_t> buffer(cbHashSize);
	if (!CryptGetHashParam(hHash, HP_HASHVAL, reinterpret_cast<BYTE*>(&buffer[0]), &cbHashSize, 0))
	{
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return {};
	}
	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);
	return buffer;
}
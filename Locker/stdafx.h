#pragma once
#define _CRT_SECURE_NO_WARNINGS 1
#include <spdlog/tweakme.h>
#include <iostream>
#include <Windows.h>
#include <conio.h>
#include <spdlog/spdlog.h>
#include <fmt/xchar.h>
#include <lazy_importer.hpp>
#include "XorString.h"
#include <boost/filesystem.hpp>
#include <benchmark/benchmark.h>
#include <chrono>
#include <fstream>
#include <boost/serialization/map.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <filesystem>
#include <cryptopp/aes.h>
#include <cryptopp/eccrypto.h>
#include <cryptopp/osrng.h>
#include <cryptopp/oids.h>
#include <cryptopp/hex.h>
#include <cryptopp/filters.h>
#include <cryptopp/modes.h>
#include <cryptopp/files.h>
#include <clipboardxx.hpp>

namespace boost {
	namespace serialization {
		template<unsigned int N>
		struct Serialize
		{
			template<class Archive, typename... Args>
			static void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version)
			{
				ar& std::get<N - 1>(t);
				Serialize<N - 1>::serialize(ar, t, version);
			}
		};

		template<>
		struct Serialize<0>
		{
			template<class Archive, typename... Args>
			static void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version)
			{
				(void)ar;
				(void)t;
				(void)version;
			}
		};

		template<class Archive, typename... Args>
		void serialize(Archive& ar, std::tuple<Args...>& t, const unsigned int version)
		{
			Serialize<sizeof...(Args)>::serialize(ar, t, version);
		}
	}
}
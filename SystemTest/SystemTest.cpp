#include "pch.h"
#include "CppUnitTest.h"
#include "../WinUtil/SystemUUID.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace std;

namespace sys = WinUtil::System;

namespace SystemTest
{
	TEST_CLASS(SystemUuuidTest)
	{
	public:

		const array<BYTE, 16> uuid = { 0x33, 0x22, 0x11, 0x00, 0x55, 0x44, 0x77, 0x66, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }; 

		TEST_METHOD(UUID_with_all_zeros_is_invalid)
		{
			array<BYTE, 16> uuid{};
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), uuid.size()));
		}

		TEST_METHOD(UUID_with_all_FF_is_invalid)
		{
			array<BYTE, 16> uuid;
			fill(uuid.begin(), uuid.end(), 0xFF);
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), uuid.size()));
		}

		TEST_METHOD(valid_uuid_has_16_bytes)
		{
			Assert::AreEqual(static_cast<size_t>(16), uuid.size());
			Assert::IsTrue(sys::SystemUUID::is_valid_uuid(uuid.data(), 16));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 0));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 1));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 15));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 17));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 20));
			Assert::IsFalse(sys::SystemUUID::is_valid_uuid(uuid.data(), 37));
		}

		TEST_METHOD(valid_uuid_may_contain_0x00_or_0xFF)
		{
			Assert::IsTrue(sys::SystemUUID::is_valid_uuid(uuid.data(), uuid.size()));
		}

		TEST_METHOD(toString_respects_byte_orders)
		{
			Assert::AreEqual(string("00112233-4455-6677-8899-AABBCCDDEEFF"), sys::SystemUUID::raw_uuid_to_string(uuid));
		}

		TEST_METHOD(fromString_respects_byte_orders)
		{
			auto id = sys::SystemUUID::string_to_raw_uuid("00112233-4455-6677-8899-AABBCCDDEEFF");
			Assert::IsTrue(equal(uuid.begin(), uuid.end(), id.begin()));
		}

		TEST_METHOD(fromString_supports_lower_case_uuids)
		{
			auto id = sys::SystemUUID::string_to_raw_uuid("00112233-4455-6677-8899-aabbccddeeff");
			Assert::IsTrue(equal(uuid.begin(), uuid.end(), id.begin()));
		}

		TEST_METHOD(retrive_returns_same_as_wmi_call)
		{
			array<BYTE, 16> id{};
			Assert::IsTrue(sys::SystemUUID::retrieve_from_system(id));
			Assert::IsTrue(0 == system("wmic csproduct get uuid>wmi_uuid_uc16.txt"));
			auto idStr = sys::SystemUUID::raw_uuid_to_string(id);
			Assert::IsFalse(0 == system((string("find \"asd\" wmi_uuid_uc16.txt").c_str())));
			Assert::IsTrue(0 == system((string("find \"UUID\" wmi_uuid_uc16.txt").c_str())));
			Assert::IsTrue(0 == system((string("find \"") + idStr + "\" wmi_uuid_uc16.txt").c_str()));
		}
	};
}

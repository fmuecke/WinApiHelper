// Copyright 2023 Florian Muecke. All rights reserved.
//
// Helper to retrieve the systems vendors product UUID as specified by
// - https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/19_ASL_Reference/ACPI_Source_Language_Reference.html
// - https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
//
#pragma once

#include <Windows.h>
#include <vector>
#include <array>
#include <algorithm>

namespace WinUtil
{
	namespace System
	{
        class SystemUUID 
        {
        public:
            SystemUUID()
            {
            }

			SystemUUID(std::array<BYTE, 16> rawUuid) : _uuid(rawUuid)
			{}

			//SystemUUID(const char* uuidStr, uint8_t len)
			//{
                //if (len != 37)
            //}

            bool Init() 
            {
                return retrieve_from_system(_uuid);
            }

            std::string ToString() const 
			{ 
				return raw_uuid_to_string(_uuid);
			}

			std::array<BYTE, 16> GetRaw() const
			{
				return _uuid;
			}

            static std::string RetriveAsString()
            {
                SystemUUID uuid{};
                uuid.Init();
                return uuid.ToString();
            }

		private:
            static bool is_valid_uuid(const BYTE* pUuid, size_t len)
			{
				// If the value is all FFh, the ID is not currently present in the system, but it can be set. 
				// If the value is all 00h, the ID is not present in the system.
				// see section 7.2.1

				if (len != 16) return false;
				const auto isAllZero = std::all_of(pUuid, pUuid + 16, [](DWORD i) { return i == 0x00000000; });
				const auto isAllFF = std::all_of(pUuid, pUuid + 16, [](DWORD i) { return i == 0xFFFFFFFF; });

				return !isAllZero && !isAllFF;
			}

			static std::string raw_uuid_to_string(const std::array<BYTE, 16>& uuid)
			{
				// The UUID {00112233-4455-6677-8899-AABBCCDDEEFF} would thus be represented as: 
				// 33 22 11 00 55 44 77 66 88 99 AA BB CC DD EE FF. 
				// see https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf section 7.2.1

				char uuidString[37];
				snprintf(uuidString, sizeof(uuidString), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
					uuid[3], uuid[2], uuid[1], uuid[0], uuid[5], uuid[4], uuid[7], uuid[6],
					uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

				return uuidString;
			}

            static bool retrieve_from_system(std::array<BYTE, 16>& uuid)
            {
                // determine the required buffer size
                auto size = ::GetSystemFirmwareTable('RSMB', 0, nullptr, 0);

                // allocate the buffer and retrieve the SMBIOS data
                std::vector<BYTE> biosData(size, 0x00);
                if (!::GetSystemFirmwareTable('RSMB', 0, biosData.data(), biosData.size()))
                {
                    ::OutputDebugStringA("Failed to retrieve SMBIOS data.");
                    return false;
                }

                const auto system_information_structure_size = 0x19; //TODO FM: 0x1A istead of 0x19??
                const auto uuid_offset = 0x08;
                const auto uuid_length = 0x10;

                // header of the SMBIOS header according to section 6.1.2 in System Management BIOS (SMBIOS) Reference
                // https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
#pragma pack(push, 1)
                struct SMBIOSHeader {
                    BYTE type;
                    BYTE length;
                    WORD handle;
                };
#pragma pack(pop)

#pragma warning(push)
#pragma warning(disable : 4200) // MS extension empty array
                // see Microsoft API https://docs.microsoft.com/en-us/windows/win32/api/sysinfoapi/nf-sysinfoapi-getsystemfirmwaretable
                struct RawSMBIOSData
                {
                    BYTE Used20CallingMethod;
                    BYTE SMBIOSMajorVersion;
                    BYTE SMBIOSMinorVersion;
                    BYTE DmiRevision;
                    DWORD Length;
                    BYTE SMBIOSTableData[];
                };
#pragma warning(pop)

                // Go through BIOS structures
                auto pBiosData = reinterpret_cast<const RawSMBIOSData*>(biosData.data());
                while (pBiosData->SMBIOSTableData < pBiosData->SMBIOSTableData + pBiosData->Length)
                {
                    const BYTE* pNext{ nullptr };
                    auto header = reinterpret_cast<const SMBIOSHeader*>(pBiosData);

                    // Check if it's the System Information structure (type 1) and has a UUID (see section 7.2)
                    if (header->type == 0x01 && header->length >= system_information_structure_size)
                    {
                        const BYTE* pUuid = reinterpret_cast<const BYTE*>(pBiosData) + uuid_offset;

                        if (is_valid_uuid(pUuid, uuid_length))
                        {
                            std::copy_n(pUuid, 16, uuid.data());
                            return true;
                        }
                        break;
                    }

                    // Move to the next SMBIOS structure...

                    // skip header
                    pNext = reinterpret_cast<const BYTE*>(pBiosData) + header->length;

                    // skip data part (end marked by 0x00,0x00)
                    while (pNext < pBiosData->SMBIOSTableData + pBiosData->Length && (*pNext != 0 || *(pNext + 1) != 0)) ++pNext;

                    // skip end mark
                    pNext += 2;

                    pBiosData = reinterpret_cast<const RawSMBIOSData*>(pNext);
                }

                ::OutputDebugStringA("System UUID not found.");
                return false;
            }

			std::array<BYTE, 16> _uuid{};
        };

	}
}

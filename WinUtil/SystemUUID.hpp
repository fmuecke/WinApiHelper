// Copyright 2023 Florian Muecke. All rights reserved.
//
// Helper to retrieve the systems vendors product UUID as specified by
// - https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/19_ASL_Reference/ACPI_Source_Language_Reference.html
// - https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
//
// returns the same as "wmic csproduct get uuid"
#pragma once

#include <Windows.h>
#include <vector>
#include <array>
#include <algorithm>

namespace WinUtil
{
	namespace System
	{
        namespace SystemUUID 
        {
            static bool is_valid_uuid(const BYTE* pUuid, size_t len)
			{
				// If the value is all FFh, the ID is not currently present in the system, but it can be set. 
				// If the value is all 00h, the ID is not present in the system.
				// see section 7.2.1

				if (len != 16) return false;
				const auto isAllZero = std::all_of(pUuid, pUuid + 16, [](BYTE i) { return i == 0x00; });
				const auto isAllFF = std::all_of(pUuid, pUuid + 16, [](BYTE i) { return i == 0xFF; });

				return !isAllZero && !isAllFF;
			}

			static std::string raw_uuid_to_string(const std::array<BYTE, 16>& uuid)
			{
				// The UUID {00112233-4455-6677-8899-AABBCCDDEEFF} is represented as: 
				// 33 22 11 00 55 44 77 66 88 99 AA BB CC DD EE FF. 
				// see https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf section 7.2.1

				char uuidString[37];
				snprintf(uuidString, sizeof(uuidString), "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
					uuid[3], uuid[2], uuid[1], uuid[0], uuid[5], uuid[4], uuid[7], uuid[6],
					uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14], uuid[15]);

				return uuidString;
			}

            static std::array<BYTE, 16> string_to_raw_uuid(std::string uuidStr)
            {
                std::array<BYTE, 16> byteArray{};
                if (uuidStr.length() == 36 && 4 == std::count(uuidStr.begin(), uuidStr.end(), '-'))
                {
                    // remove separator chars
                    uuidStr.erase(std::remove(uuidStr.begin(), uuidStr.end(), '-'), uuidStr.end());

                    auto _hexCharToByte = [](char c)
                    {
                        return
                            (c >= '0' && c <= '9') ? static_cast<unsigned char>(c - '0') :
                            (c >= 'a' && c <= 'f') ? static_cast<unsigned char>(c - 'a' + 10) :
                            (c >= 'A' && c <= 'F') ? static_cast<unsigned char>(c - 'A' + 10) : 0;
                    };

                    // The UUID {00112233-4455-6677-8899-AABBCCDDEEFF} needs to be represented as: 
                    // 33 22 11 00 55 44 77 66 88 99 AA BB CC DD EE FF. 
                    byteArray[0] = (_hexCharToByte(uuidStr[ 6]) << 4) | _hexCharToByte(uuidStr[7]);
                    byteArray[1] = (_hexCharToByte(uuidStr[ 4]) << 4) | _hexCharToByte(uuidStr[5]);
                    byteArray[2] = (_hexCharToByte(uuidStr[ 2]) << 4) | _hexCharToByte(uuidStr[3]);
                    byteArray[3] = (_hexCharToByte(uuidStr[ 0]) << 4) | _hexCharToByte(uuidStr[1]);
                    byteArray[4] = (_hexCharToByte(uuidStr[10]) << 4) | _hexCharToByte(uuidStr[11]);
                    byteArray[5] = (_hexCharToByte(uuidStr[ 8]) << 4) | _hexCharToByte(uuidStr[9]);
                    byteArray[6] = (_hexCharToByte(uuidStr[14]) << 4) | _hexCharToByte(uuidStr[15]);
                    byteArray[7] = (_hexCharToByte(uuidStr[12]) << 4) | _hexCharToByte(uuidStr[13]);
                    
                    for (std::size_t i = 16; i < uuidStr.size(); i += 2) {
                        BYTE byte = (_hexCharToByte(uuidStr[i]) << 4) | _hexCharToByte(uuidStr[i + 1]);
                        byteArray[i / 2] = byte;
                    }
                }

                return byteArray;
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

            static std::string GetAsString()
            {
                std::array<BYTE, 16> uuid{};
                if (!retrieve_from_system(uuid)) return "";
                return raw_uuid_to_string(uuid);
            }
        }
	}
}

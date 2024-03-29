// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <memory>

#pragma comment(lib, "Advapi32.lib")

namespace WinUtil
{
	class Registry
	{

	public:
		enum class Mode : REGSAM
		{
			Read = KEY_READ,
			Write = KEY_WRITE
		};

		enum class ValueType
		{
			none                       = 0,   // No value type
			sz                         = 1,   // Unicode nul terminated string
			expand_sz                  = 2,   // Unicode nul terminated string
			// (with environment variable references)
			binary                     = 3,   // Free form binary
			dword                      = 4,   // 32-bit number
			dword_little_endian        = 4,   // 32-bit number (same as REG_DWORD)
			dword_big_endian           = 5,   // 32-bit number
			link                       = 6,   // Symbolic Link (unicode)
			multi_sz                   = 7,   // Multiple Unicode strings
			resource_list              = 8,   // Resource list in the resource map
			full_resource_descriptor   = 9,  // Resource list in the hardware description
			resource_requirements_list = 10,
			qword                      = 11,  // 64-bit number
			qword_little_endian        = 11,   // 64-bit number (same as REG_QWORD)
		};
		
		Registry() {}
		~Registry() 
		{
			Close();
		}

		HKEY Key() const noexcept  { return _hKey; }

		DWORD Open(HKEY hKey, std::wstring const& subKey, Mode mode) noexcept
		{
			Close();
			return ::RegOpenKeyExW(hKey, subKey.c_str(), 0, static_cast<REGSAM>(mode), &_hKey);
		}
		
		DWORD EnumKeys(std::vector<std::wstring>& subKeys) noexcept
		{
			if (_hKey == 0) return ERROR_INVALID_HANDLE; 
			DWORD nSubKey = 0;
			DWORD maxSubKeyLen = 0;
			DWORD ret = ::RegQueryInfoKey(_hKey, NULL, NULL, 0, &nSubKey, &maxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);
			if (ret != ERROR_SUCCESS) return ret;
			auto buffer = std::vector<wchar_t>(maxSubKeyLen + 1, 0);
			DWORD index = 0;
			DWORD keyLen = maxSubKeyLen + 1;
			std::vector<std::wstring> values;
			ret = ::RegEnumKeyExW(_hKey, index++, buffer.data(), &keyLen, 0, NULL, NULL, NULL);
			while (ret == ERROR_SUCCESS)
			{
				values.push_back(std::wstring(&buffer[0], keyLen));
				keyLen = maxSubKeyLen + 1;
				ret = ::RegEnumKeyExW(_hKey, index++, buffer.data(), &keyLen, 0, NULL, NULL, NULL);
			}
		
			if (ret != ERROR_NO_MORE_ITEMS) return ret;
			subKeys.swap(values);
			return ERROR_SUCCESS;
		}

		DWORD TryReadString(std::wstring const& name, std::wstring& value) noexcept
		{
			if (_hKey == 0) return ERROR_INVALID_HANDLE;
            DWORD requiredSize = 0;
			auto result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, nullptr, &requiredSize);
			if (result != ERROR_SUCCESS) return result;

			requiredSize += sizeof(wchar_t);  // additional termination char
			std::unique_ptr<BYTE[]> pBuffer(new BYTE[requiredSize]);
			::SecureZeroMemory(pBuffer.get(), requiredSize);
			result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, pBuffer.get(), &requiredSize);
			if (result == ERROR_SUCCESS)
			{
				[[suppress(type.1)]] // suppress reinterpret_cast
				{
					value.assign(reinterpret_cast<wchar_t*>(pBuffer.get()));
				}
			}

			return result;
		}

		static bool TryReadString(HKEY hKey, std::wstring const& subKey, std::wstring const& name, std::wstring& value) noexcept
		{
			Registry reg;
			if (reg.Open(hKey, subKey, Mode::Read) != ERROR_SUCCESS) return false;
			return reg.TryReadString(name, value) == ERROR_SUCCESS;
		}

		DWORD TryReadDword(std::wstring const& name, DWORD& value) noexcept
		{
			if (_hKey == 0) return ERROR_INVALID_HANDLE;
			DWORD resultVal = 0;
			DWORD size = sizeof(resultVal);
			DWORD result{ 0 };
			[[suppress(type.1)]] // suppress reinterpret_cast
			{
				result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, reinterpret_cast<BYTE*>(&resultVal), &size);
			}
			if (result != ERROR_SUCCESS) return result;

			value = resultVal;
			return result;
		}

		static bool TryReadDword(HKEY hKey, std::wstring const& subKey, std::wstring const& name, DWORD& value) noexcept
		{
			Registry reg;
			if (reg.Open(hKey, subKey, Mode::Read) != ERROR_SUCCESS) return false;
			return reg.TryReadDword(name, value) == ERROR_SUCCESS;
		}
	
	private:
		void Close() noexcept
		{
			if (_hKey != 0)
			{
				::RegCloseKey(_hKey);
				_hKey = 0;
			}
		}
	
        HKEY _hKey{ 0 };
	};

}

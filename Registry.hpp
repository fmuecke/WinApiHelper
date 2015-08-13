#pragma once
#include <Windows.h>
#include <string>
#include <memory>

#pragma comment(lib, "Advapi32.lib")

namespace WinAid
{
	class Registry
	{
		HKEY _hKey = 0;
		void Close()
		{
			if (_hKey != 0)
			{
				::RegCloseKey(_hKey);
				_hKey = 0;
			}
		}

	public:
		enum class Mode
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

		DWORD Open(HKEY hKey, std::wstring const& subKey, Mode mode)
		{
			Close();
			return ::RegOpenKeyExW(hKey, subKey.c_str(), 0, static_cast<REGSAM>(mode), &_hKey);
		}

		DWORD TryReadString(std::wstring const& name, std::wstring& value)
		{
			DWORD requiredSize = 0;
			auto result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, nullptr, &requiredSize);
			if (result != ERROR_SUCCESS) return result;

			requiredSize += sizeof(wchar_t);  // additional termination char
			std::unique_ptr<BYTE[]> pBuffer(new BYTE[requiredSize]);
			::ZeroMemory(pBuffer.get(), requiredSize);
			result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, pBuffer.get(), &requiredSize);
			if (result == ERROR_SUCCESS)
			{
				value.swap(std::wstring(reinterpret_cast<wchar_t*>(pBuffer.get()), static_cast<size_t>(requiredSize / sizeof(wchar_t))));
			}

			return result;
		}

		static bool TryReadString(HKEY hKey, std::wstring const& subKey, std::wstring const& name, std::wstring& value)
		{
			Registry reg;
			if (reg.Open(hKey, subKey, Mode::Read) != ERROR_SUCCESS) return false;
			return reg.TryReadString(name, value) == ERROR_SUCCESS;
		}

		DWORD TryReadDword(std::wstring const& name, DWORD& value)
		{
			DWORD resultVal = 0;
			DWORD size = sizeof(resultVal);
			auto result = ::RegQueryValueExW(_hKey, name.c_str(), 0, nullptr, reinterpret_cast<BYTE*>(&resultVal), &size);
			if (result != ERROR_SUCCESS) return result;

			value = resultVal;
			return result;
		}

		static bool TryReadDword(HKEY hKey, std::wstring const& subKey, std::wstring const& name, DWORD& value)
		{
			Registry reg;
			if (reg.Open(hKey, subKey, Mode::Read) != ERROR_SUCCESS) return false;
			return reg.TryReadDword(name, value) == ERROR_SUCCESS;
		}
	};

}

#pragma once

#include "Registry.hpp"
#include "Environment.hpp"
#include <string>


namespace WinAid
{
	namespace WindowsVersion
	{
		static std::wstring GetLocaleName()
		{
			std::wstring value;
			Registry::TryReadString(HKEY_USERS, L"S-1-5-18\\Control Panel\\International", L"LocaleName", value);
			return value;
		}

		static bool IsX64()
		{
			std::wstring arch;
			return Environment::ExpandStrings(L"%PROCESSOR_ARCHITECTURE%", arch) && arch == L"AMD64";
		}

		struct Info
		{
			std::wstring CurrentBuildNumber;
			std::wstring BuildLabEx;
			std::wstring CurrentVersion;
			std::wstring EditionId;
			std::wstring ProductName;
			std::wstring Architecture;
			std::wstring Language;
			DWORD CurrentMajorVersionNumber;
			DWORD CurrentMinorVersionNumber;

			std::wstring ToString()
			{
				// will produce somethin like "Windows 10 Pro x64 EN (Build 10240)"
				return ProductName + L" " + Architecture + L" " + Language + L" (Build " + CurrentBuildNumber + L")";
			}

			DWORD Init()
			{
				Registry reg;
				auto result = reg.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", Registry::Mode::Read);
				if (result != ERROR_SUCCESS)
				{
					return result;
				}
				reg.TryReadString(L"CurrentBuildNumber", CurrentBuildNumber);
				reg.TryReadString(L"BuildLabEx", BuildLabEx);
				reg.TryReadString(L"CurrentVersion", CurrentVersion);
				reg.TryReadString(L"EditionId", EditionId);
				reg.TryReadString(L"ProductName", ProductName);
				reg.TryReadDword(L"CurrentMajorVersionNumber", CurrentMajorVersionNumber);
				reg.TryReadDword(L"CurrentMinorVersionNumber", CurrentMinorVersionNumber);
				Architecture = IsX64() ? L"x64" : L"x86";
				Language = GetLocaleName().substr(0, 2);

				return ERROR_SUCCESS;
			}
		};
	};
}

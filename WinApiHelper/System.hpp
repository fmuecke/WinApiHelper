#pragma once

#include "Registry.hpp"
#include "Environment.hpp"
#include "Locale.hpp"
#include "Security.hpp"

#include <string>
#include <vector>

namespace WinApiHelper
{
	namespace System
	{
		static bool IsX64()
		{
			SYSTEM_INFO si = { 0 };
			::GetNativeSystemInfo(&si);		
			return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
		}

		bool IsWow64()
		{
			typedef BOOL(WINAPI *ISWOW64PROCESS) (HANDLE, PBOOL);
			ISWOW64PROCESS fnIsWow64Process = 
				(ISWOW64PROCESS)::GetProcAddress(GetModuleHandleW(L"kernel32.dll"), "IsWow64Process");
			BOOL bIsWow64 = false;
			if (NULL != fnIsWow64Process)
			{
				if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
				{
					return false;
				}
			}

			return bIsWow64 != 0;
		}

		static std::wstring GetDefaultLocaleName()
		{
			std::wstring locale;
			Locale::LCIDToLocaleName(GetSystemDefaultLCID(), locale);
			return locale;
		}
		
		// Retrieves the account names (domain\user) for all local accounts
		static DWORD GetLocalAccounts(std::vector<std::wstring>& values)
		{
			Registry reg;
			DWORD ret = reg.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", Registry::Mode::Read);
			if (ret != ERROR_SUCCESS) return ret;
			std::vector<std::wstring> profileSids;
			ret = reg.EnumKeys(profileSids);
			if (ret != ERROR_SUCCESS) return ret;
			
			std::vector<std::wstring> _accounts;
			for (auto const& sidStr : profileSids)
			{
				std::wstring user;
				std::wstring domain;
				ret = Security::SidToAccountName(sidStr, user, domain);
				if (ret != ERROR_SUCCESS) return ret;
				_accounts.push_back(domain + L"\\" + user);
			}

			values.swap(_accounts);
			return ERROR_SUCCESS;
		}		

		struct WindowsVersion
		{
			std::wstring CurrentBuildNumber;
			std::wstring BuildLabEx;
			std::wstring CurrentVersion;
            std::wstring CSDBuildNumber; // up to windows 7
            std::wstring CSDVersion; // up to windows 7
			std::wstring EditionId;
			std::wstring ProductName;
			std::wstring Architecture;
			std::wstring Language;
			DWORD CurrentMajorVersionNumber = 0;
			DWORD CurrentMinorVersionNumber = 0;

			std::wstring ToString()
			{
				// will produce something like "Windows 10 Pro x64 en"
				return ProductName + L" " + Architecture + L" " + Language.substr(0,2);
			}

			std::wstring ToExtendedString()
			{
				// will produce somethin like "Windows 10 Pro x64 en-US (Build 10240)"
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
				reg.TryReadDword(L"CurrentMajorVersionNumber", CurrentMajorVersionNumber);
				reg.TryReadDword(L"CurrentMinorVersionNumber", CurrentMinorVersionNumber);	
                reg.TryReadString(L"CSDbuildNumber", CSDBuildNumber);
                reg.TryReadString(L"CSDVersion", CSDVersion);
                if (reg.TryReadString(L"BuildLabEx", BuildLabEx) != NO_ERROR)
                {
                    reg.TryReadString(L"BuildLab", BuildLabEx);
                }
				reg.TryReadString(L"CurrentVersion", CurrentVersion);
				reg.TryReadString(L"EditionId", EditionId);
				reg.TryReadString(L"ProductName", ProductName);
				if (CurrentMajorVersionNumber > 0)
				{
					CurrentVersion = std::to_wstring(CurrentMajorVersionNumber) + L"." + std::to_wstring(CurrentMinorVersionNumber);
				}
				else
				{
					reg.TryReadString(L"CurrentVersion", CurrentVersion);
				}

				Architecture = IsX64() ? L"x64" : L"x86";
				Language = GetDefaultLocaleName();// .substr(0, 2);

				return ERROR_SUCCESS;
			}
		};
	}
}

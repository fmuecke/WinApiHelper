// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once

#include "Registry.hpp"
#include "Environment.hpp"
#include "Locale.hpp"
#include "Security.hpp"
#include "UserProfile.hpp"

#include <string>
#include <vector>
#include <array>
#include <stdexcept>
#include <system_error>

namespace WinUtil
{
	namespace System
	{
        class SysError : public std::runtime_error 
        {
        public:
            explicit SysError(DWORD code) 
                : std::runtime_error(std::error_code(code, std::system_category()).message())
                , _code{code}
            {}
            DWORD Value() const noexcept { return _code; }
        private:
            DWORD _code = 0;
        };

        static bool IsX64()
		{
			SYSTEM_INFO si = { 0 };
			::GetNativeSystemInfo(&si);		
			return si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64;
		}

		static bool IsWow64()
		{
			auto hModule = ::GetModuleHandleW(L"kernel32.dll");
			if (!hModule) return false;

			using IsWow64ProcessFun = BOOL(WINAPI*)(HANDLE, PBOOL);
			IsWow64ProcessFun fnIsWow64Process{ nullptr };
			[[suppress(type.1)]] // suppress reinterpret_cast
			{
				fnIsWow64Process = reinterpret_cast<IsWow64ProcessFun>(::GetProcAddress(hModule, "IsWow64Process"));
			}

			BOOL isWow64{ false };
			if (NULL != fnIsWow64Process)
			{
				if (!fnIsWow64Process(::GetCurrentProcess(), &isWow64))
				{
					return false;
				}
			}

			return isWow64 != 0;
		}

		static std::wstring GetDefaultLocaleName()
		{
			std::wstring locale;
            Locale::LCIDToLocaleName(::GetSystemDefaultUILanguage(), locale);
			return locale;
		}
		
		// Retrieves the account names (domain\user) for all local accounts
        std::vector<UserProfile> GetLocalProfiles()
		{
            Registry reg;
			DWORD ret = reg.Open(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList", Registry::Mode::Read);
            if (ret != ERROR_SUCCESS) throw SysError(ret);

            std::vector<std::wstring> profileSids;
			ret = reg.EnumKeys(profileSids);
			if (ret != ERROR_SUCCESS) throw SysError(ret);
			
			std::vector<UserProfile> accounts;
			for (auto const& sidStr : profileSids)
			{
                UserProfile profile{};
				profile.sid = sidStr;
				Registry::TryReadString(reg.Key(), sidStr, L"ProfileImagePath", profile.path);
				/*DWORD ret = */Security::SidToAccountName(sidStr, profile.name, profile.domain);
                accounts.push_back(profile);
			}

			return accounts;
		}		

		struct WindowsVersion
		{
            std::wstring CurrentBuildNumber{};
            std::wstring BuildLabEx{};
            std::wstring CurrentVersion{};
            std::wstring CSDBuildNumber{}; // up to windows 7
            std::wstring ServicePack{}; // up to windows 7
			std::wstring EditionId{};
			std::wstring ProductName{};
			std::wstring Architecture{};
			std::wstring Language{};
			std::wstring ReleaseId{};
			DWORD UBR = 0;
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
                if (ServicePack.empty())
                {
                    return ProductName + L" " + Architecture + L" " + Language + L" (Build " + CurrentBuildNumber + L")";
                }
                else
                {
                    return ProductName + L" " + ServicePack + L" " + Architecture + L" " + Language + L" (Build " + CurrentBuildNumber + L")";
                }
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
				reg.TryReadDword(L"UBR", UBR);
				reg.TryReadString(L"ReleaseId", ReleaseId);
                reg.TryReadString(L"CSDbuildNumber", CSDBuildNumber);
                reg.TryReadString(L"CSDVersion", ServicePack);
                auto pos = ServicePack.find_last_not_of(L"Service Pack ");
                if (pos != std::wstring::npos)
                {
                    ServicePack = L"SP" + ServicePack.substr(pos);
                }
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

				Architecture = IsX64() ? std::wstring(L"x64") : std::wstring(L"x86");
				Language = GetDefaultLocaleName();

				return ERROR_SUCCESS;
			}
		};

		// Returns one of these: FAT, FAT32, NTFS, HPFS, CDFS, UDF, NWFS, exFAT
		static std::wstring GetFileSystemType(std::wstring const& volume)
		{
			auto szFileSystemName = std::array<wchar_t, MAX_PATH + 1>{ 0 };
			auto const& pVolume = volume.empty() ? nullptr : volume.c_str();

			if (::GetVolumeInformationW(pVolume, nullptr, 0, nullptr, nullptr, nullptr, szFileSystemName.data(), szFileSystemName.size()))
			{
				return std::wstring(szFileSystemName.data());
			}
			return std::wstring();
		}
	}
}

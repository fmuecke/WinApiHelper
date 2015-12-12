// Copyright 2015 Florian Muecke. All rights reserved.
#include <iostream>
#include <vector>
#include <string>
#include <system_error>
#include <filesystem>
#include <memory>
#include <io.h>
#include <fcntl.h>
#include "../../WinUtil/System.hpp"
#include "../../WinUtil/Security.hpp"

using namespace std;
using namespace std::tr2;
using namespace WinUtil;

static wchar_t const * const uninstallStr = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

struct UninstallData
{
	wstring userProfile{};
	wstring key{};
	wstring displayName{};
	wstring displayVersion{};
	wstring publisher{};

	wstring ToText() const 
	{
		return userProfile + L": DisplayName=" + displayName + L", Publisher=" + publisher + L", DisplayVersion=" + displayVersion + L", Key=" + key;
	}
};

static UninstallData GetUninstallData(HKEY const& hKey, wstring const& key)
{
	UninstallData data{};
	data.key = key;
	Registry::TryReadString(hKey, key, L"DisplayName", data.displayName);
	Registry::TryReadString(hKey, key, L"DisplayVersion", data.displayVersion);
	Registry::TryReadString(hKey, key, L"Publisher", data.publisher);
	return data;
}

static bool SetProcRegAccessPrivs(bool doSet)
{
	HANDLE hToken;
	if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		cerr << "Error opening process token: " << ::GetLastError() << endl;
		return false;
	}

	LUID restorePriv{};
	LUID backupPriv{};
	if (!::LookupPrivilegeValue(nullptr, SE_RESTORE_NAME, &restorePriv) ||
		!::LookupPrivilegeValue(nullptr, SE_BACKUP_NAME, &backupPriv))
	{
		cerr << "Error getting privilege values" << endl;
		return false;
	}

	// create buffer with enough space for token privileges and an additional LUID with attributes
	[[suppress(bounds.2)]]
	{
		auto buffer = std::vector<byte>(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES), 0);
		[[suppress(type.1)]] // suppress reinterpret_cast
		{
			TOKEN_PRIVILEGES* pTokenPrivileges = reinterpret_cast<TOKEN_PRIVILEGES*>(buffer.data());
			pTokenPrivileges->PrivilegeCount = 2;
			pTokenPrivileges->Privileges[0].Luid = restorePriv;
			pTokenPrivileges->Privileges[0].Attributes = doSet ? SE_PRIVILEGE_ENABLED : 0;
			pTokenPrivileges->Privileges[1].Luid = backupPriv;
			pTokenPrivileges->Privileges[1].Attributes = doSet ? SE_PRIVILEGE_ENABLED : 0;

			if (!::AdjustTokenPrivileges(hToken, false, pTokenPrivileges, 0, nullptr, nullptr))
			{
				auto err = ::GetLastError();
				cerr << "Error setting privilege values: " << err << endl;
				return false;
			}
		}
	}

	return true;
}

static vector<UninstallData> ScanUserKey(HKEY appKey, wstring const& subKey, WinUtil::UserProfile const& profile)
{
	vector<UninstallData> result;
	Registry reg;
	auto openResult = reg.Open(appKey, subKey, Registry::Mode::Read);
	if (openResult == ERROR_FILE_NOT_FOUND)
	{
		// no software subkey for this user
		return result;
	}

	if (openResult == ERROR_SUCCESS)
	{
		vector<wstring> subKeys;
		reg.EnumKeys(subKeys);
		for (auto const& key : subKeys)
		{
			auto data = GetUninstallData(reg.Key(), key);
			data.userProfile = profile.GetFullAccountName();
			result.push_back(std::move(data));
		}
	}
	else
	{
		auto err = error_code(openResult, system_category());
		cerr << "error: " << err.message();
	}

	return result;
}

int main()
{
    _setmode(_fileno(stdout), _O_U16TEXT);

    SetProcRegAccessPrivs(true);
	bool verbose = false;

	vector<UserProfile> profiles;
	System::GetLocalProfiles(profiles);

	vector<UninstallData> uninstallData;
	using RegLoadAppKeyFun = LONG(WINAPI*)(LPCTSTR, PHKEY, REGSAM, DWORD, DWORD);
	auto hModule = GetModuleHandleW(L"Advapi32.dll");
	if (!hModule) exit(1);
	RegLoadAppKeyFun fnRegLoadAppKey{ nullptr };
	[[suppress(type.1)]] // suppress reinterpret_cast
	{
		fnRegLoadAppKey = reinterpret_cast<RegLoadAppKeyFun>(::GetProcAddress(hModule, "RegLoadAppKeyW"));
	}
	if (fnRegLoadAppKey)
	{
		for (auto const& profile : profiles)
		{
			if (profile.path.empty()) continue;
			auto path = profile.path;
			path.append(path.back() == L'\\' ? wstring(L"NTUSER.DAT") : wstring(L"\\NTUSER.DAT"));
			if (!sys::exists(sys::path(path))) continue;

			HKEY appKey;
			DWORD loadResult = fnRegLoadAppKey(path.c_str(), &appKey, KEY_ALL_ACCESS, REG_PROCESS_APPKEY, 0);
			if (ERROR_SUCCESS == loadResult || ERROR_SHARING_VIOLATION == loadResult)
			{
				auto subKey = ERROR_SUCCESS == loadResult ? wstring(uninstallStr) : profile.sid + L"\\" + uninstallStr;
				if (ERROR_SHARING_VIOLATION == loadResult) appKey = HKEY_USERS;
				auto userData = ScanUserKey(appKey, subKey, profile);
				if (verbose) wcout << profile.GetFullAccountName() << L": " << userData.size() << L" found" << endl;
				uninstallData.insert(cend(uninstallData), cbegin(userData), cend(userData));

				::RegCloseKey(appKey);
			}
			else
			{
				if (ERROR_BADDB == loadResult) //ERROR_PRIVILEGE_NOT_HELD
				{
					// really load the hive
					loadResult = ::RegLoadKeyW(HKEY_USERS, profile.name.c_str(), path.c_str());
					if (loadResult == ERROR_SUCCESS)
					{
						auto subKey = "";
						auto userData = ScanUserKey(HKEY_USERS, profile.name + L"\\" + uninstallStr, profile);
						if (verbose) wcout << profile.GetFullAccountName() << L": " << userData.size() << L" found" << endl;
						uninstallData.insert(cend(uninstallData), cbegin(userData), cend(userData));

						::RegUnLoadKeyW(HKEY_USERS, profile.name.c_str());
						continue;
					}
				}
				wcerr << profile.name << L": ";
				auto err = error_code(loadResult, system_category());
				cerr << "error " << err.value() << ": " << err.message();
			}
		}
	}
	else
	{
		cerr << "platform not supported" << endl;
		exit(ERROR_INVALID_FUNCTION);
	}

    wcout << uninstallData.size() << L" found\n";
	for (auto const& data : uninstallData)
	{
        auto result = data.ToText();
        wcout << result << endl;
	}

	return 0;
}

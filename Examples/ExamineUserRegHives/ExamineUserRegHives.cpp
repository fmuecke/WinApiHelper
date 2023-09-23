// Copyright 2015 Florian Muecke. All rights reserved.
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>
#include <memory>
#include <io.h>
#include <fcntl.h>
#include "../../WinUtil/System.hpp"
#include "../../WinUtil/Security.hpp"
#include <windows.h>

using namespace std;
using namespace WinUtil;

static wchar_t const * const uninstallStr = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

static string ToAnsi(const wstring& wstr)
{
    auto requiredLen = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<DWORD>(wstr.size()), nullptr, 0, nullptr, nullptr);
    string r;
    r.resize(requiredLen, 0);
    auto actualLen = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), static_cast<DWORD>(wstr.size()), &r[0], requiredLen, nullptr, nullptr);
    return r;
}

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
		auto buffer = std::vector<unsigned char>(sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES), 0);
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
		auto err = System::SysError(openResult);
		cerr << "error: " << err.what();
	}

	return result;
}

int main()
{
    try
    {
        //_setmode(_fileno(stdout), _O_WTEXT);

        SetProcRegAccessPrivs(true);

	    vector<UserProfile> profiles = System::GetLocalProfiles();
        cout << "Profiles found: " << profiles.size() << "\n";
        for (auto const& profile : profiles)
        {
            cout << "  " << ToAnsi(profile.GetFullAccountName()) << "\n";
        }

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
			    if (!filesystem::exists(filesystem::path(path))) continue;

			    HKEY appKey;
			    DWORD loadResult = fnRegLoadAppKey(path.c_str(), &appKey, KEY_ALL_ACCESS, REG_PROCESS_APPKEY, 0);
			    if (ERROR_SUCCESS == loadResult || ERROR_SHARING_VIOLATION == loadResult)
			    {
				    auto subKey = ERROR_SUCCESS == loadResult ? wstring(uninstallStr) : profile.sid + L"\\" + uninstallStr;
				    if (ERROR_SHARING_VIOLATION == loadResult) appKey = HKEY_USERS;
				    auto userData = ScanUserKey(appKey, subKey, profile);
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
						    uninstallData.insert(cend(uninstallData), cbegin(userData), cend(userData));

						    ::RegUnLoadKeyW(HKEY_USERS, profile.name.c_str());
						    continue;
					    }
				    }
				    cerr << ToAnsi(profile.name) << ": ";
				    auto err = System::SysError(loadResult);
				    cerr << "error " << err.Value() << ": " << err.what();
			    }
		    }
	    }
	    else
	    {
		    cerr << "platform not supported" << endl;
		    exit(ERROR_INVALID_FUNCTION);
	    }

        cout << "\nPrograms found: " << uninstallData.size() << "\n";
	    for (auto const& data : uninstallData)
	    {
            auto result = ToAnsi(data.ToText());
            cout << "  " << result << endl;
	    }

        return 0;
    }
    catch (WinUtil::System::SysError const& err)
    {
        cerr << err.what();
        return err.Value();
    }
    catch (std::exception const& err)
    {
        cerr << err.what();
    }
    catch (...)
    {
        cerr << "unknown error";
    }

	return 1;
}

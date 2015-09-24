#include <iostream>
#include <vector>
#include <string>
#include <system_error>
#include <filesystem>
#include "../../WinApiHelper/System.hpp"
#include "../../WinApiHelper/Security.hpp"

using namespace std;
using namespace std::tr2;
using namespace WinApiHelper;

static wchar_t const * const uninstallStr = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

struct UninstallData
{
	wstring userProfile;
	wstring key;
	wstring displayName;
	wstring displayVersion;
	wstring publisher;

	wstring ToText() const 
	{
		return userProfile + L": " + key + L" / " + publisher + L" /" + displayName + L" / " + displayVersion;
	}
};

static UninstallData GetUninstallData(HKEY const& hKey, wstring const& key)
{
	UninstallData data;
	data.key = key;
	Registry::TryReadString(hKey, key, L"DisplayName", data.displayName);
	Registry::TryReadString(hKey, key, L"DisplayVersion", data.displayVersion);
	Registry::TryReadString(hKey, key, L"Publisher", data.publisher);
	return data;
}

int main()
{
	vector<UserProfile> profiles;
	System::GetLocalProfiles(profiles);

	vector<UninstallData> uninstallData;
	typedef LONG(WINAPI *REGLOADAPPKEY)(LPCTSTR, PHKEY, REGSAM, DWORD, DWORD);
	REGLOADAPPKEY fnRegLoadAppKey = (REGLOADAPPKEY)::GetProcAddress(GetModuleHandleW(L"Advapi32.dll"), "RegLoadAppKeyW");
	if (fnRegLoadAppKey)
	{
		for (auto const& profile : profiles)
		{
			if (profile.path.empty()) continue;
			auto path = profile.path;
			path.append(path.back() == L'\\' ? L"NTUSER.DAT" : L"\\NTUSER.DAT");
			if (!sys::exists(sys::path(path))) continue;

			HKEY appKey;
			DWORD result = fnRegLoadAppKey(path.c_str(), &appKey, KEY_ALL_ACCESS, REG_PROCESS_APPKEY, 0);
			if (ERROR_SUCCESS == result)
			{
				wcout << profile.name << ": success" << endl;

				Registry reg;
				auto success = reg.Open(appKey, wstring(uninstallStr), Registry::Mode::Read);
				vector<wstring> subKeys;
				reg.EnumKeys(subKeys);
				for (auto const& subKey : subKeys)
				{
					auto data = GetUninstallData(reg.Key(), subKey);
					data.userProfile = profile.GetFullAccountName();
					uninstallData.emplace_back(std::move(data));
				}
				
				wcout << profile.GetFullAccountName() << L": " << subKeys.size() << L" found" << endl;

				::RegCloseKey(appKey);
			}
			else
			{
				if (ERROR_SHARING_VIOLATION == result)
				{
					// key is already loaded
					Registry reg;
					if (ERROR_SUCCESS == reg.Open(HKEY_USERS, profile.sid + L"\\" + uninstallStr, Registry::Mode::Read))
					{
						vector<wstring> subKeys;
						reg.EnumKeys(subKeys);
						for (auto const& subKey : subKeys)
						{
							auto data = GetUninstallData(reg.Key(), subKey);
							data.userProfile = profile.GetFullAccountName();
							uninstallData.emplace_back(std::move(data));
						}

						wcout << profile.GetFullAccountName() << L": " << subKeys.size() << L" found" << endl;
					}

					continue;  
				}
				wcerr << profile.name << L": ";
				auto err = error_code(result, system_category());
				cerr << "error " << err.value() << ": " << err.message();
			}
		}
	}
	else
	{
		cerr << "platform not supported" << endl;
	}

	wcout << endl << L"items: " << endl;
	for (auto const& data : uninstallData)
	{
		wcout << data.ToText() << endl;
	}

	return 0;
}

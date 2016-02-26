#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <Userenv.h>

#pragma comment(lib, "Userenv.lib")

#include "../../WinUtil/System.hpp"
#include "../../StdHelper/string.hpp"

using namespace std;

#ifndef RANGE
#define RANGE(c)	cbegin(c), cend(c)
#endif

static wstring GetSid(vector<WinUtil::UserProfile> const& profiles, wstring name)
{
    auto pos = find_if(RANGE(profiles), [&](WinUtil::UserProfile const& p)
    {
        return StdHelper::InvariantCompare(name, p.GetFullAccountName())
			|| StdHelper::InvariantCompare(name, p.GetFullAccountNameWithAt())
            || StdHelper::InvariantCompare(name, p.name);
    });

    return pos == profiles.end() ? wstring() : pos->sid;
}

int wmain(int argc, wchar_t** argv)
{
	auto profiles = vector<WinUtil::UserProfile>();
    try
    {
        auto args = vector<wstring>(argv, argv + argc);
        if (args.size() != 2)
        {
            throw runtime_error("please specifiy DOMAIN\\user");
        }
		profiles = WinUtil::System::GetLocalProfiles();
        auto sid = GetSid(profiles, args[1]);
		if (sid.empty())
		{
			throw runtime_error("profile not found");
		}

        auto ret = ::DeleteProfileW(sid.c_str(), nullptr, nullptr);
        if (ret)
        {
            cout << "success" << endl;
        }
        else
        {
            auto code = ::GetLastError();
			
            throw error_code(code, system_category());
        }
    }
    catch (runtime_error& e)
    {
        cerr << "(c) baramundi software AG 2016" << endl;
        cerr << "Usage: DeleteUserProfile.exe user" << endl;
        cerr << "\nerror: " << e.what() << endl;

		cerr << "\nValid profiles are: \n\n";
		for (auto const& p : profiles)
		{
			wcerr << "\t" << p.GetFullAccountNameWithAt() << endl;
		}

        return 1;
    }
    catch (error_code& e)
    {
        cerr << "\nerror deleting profile: " << e.message() << endl;
    }
}
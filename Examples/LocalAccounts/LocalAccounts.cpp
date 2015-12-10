// Copyright 2015 Florian Muecke. All rights reserved.
#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "../../WinUtil/System.hpp"

using namespace std;

int main()
{
	vector<WinUtil::UserProfile> accounts;
	auto ret = WinUtil::System::GetLocalProfiles(accounts);
	if (ret != ERROR_SUCCESS)
	{
		cerr << "error code: " << ret << endl;
		exit(ret);
	}
	for (auto const& account : accounts)
	{
		wcout << account.GetFullAccountName() << endl;
	}
	
	return 0;
}
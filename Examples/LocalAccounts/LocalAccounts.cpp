#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "../../WinApiHelper/System.hpp"

using namespace std;

int main()
{
	vector<wstring> accounts;
	auto ret = WinApiHelper::System::GetLocalAccounts(accounts);
	if (ret != ERROR_SUCCESS)
	{
		cerr << "error code: " << ret << endl;
		exit(ret);
	}
	for (auto const& account : accounts)
	{
		wcout << account << endl;
	}
	
	return 0;
}
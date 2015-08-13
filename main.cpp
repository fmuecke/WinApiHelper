#include "WindowsVersion.hpp"
#include <iostream>

using namespace std;

int main()
{
	WinAid::WindowsVersion::Info sysInfo;
	sysInfo.Init();
	wcout << WinAid::WindowsVersion::GetLocaleName();

	wcout << sysInfo.ToString();
	return 0;
}
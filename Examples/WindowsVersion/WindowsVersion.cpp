#include "../../WinApiHelper/System.hpp"
#include "../../StdHelper/string.hpp"
#include <iostream>

#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;


int main()
{
	auto tokens = StdHelper::split(L": . . :", L":", false);
	
	WinApiHelper::System::WindowsVersion winVer;
	winVer.Init();

	wcout << winVer.ToExtendedString() << endl;

	return 0;
}
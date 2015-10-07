#include "../../WinUtil/System.hpp"
#include "../../WinUtil/Locale.hpp"
#include "../../StdHelper/string.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;
using namespace WinUtil;

int main()
{
	auto tokens = StdHelper::split(L": . . :", L":", false);
	System::WindowsVersion winVer;
	winVer.Init();

	wcout << winVer.ToExtendedString() << endl;
    wcout << L"Default system language: " << System::GetDefaultLocaleName() << endl;
    
    std::wstring locale;
    
    Locale::LCIDToLocaleName(::GetUserDefaultUILanguage(), locale);
    wcout << L"Current user language: " << locale << endl;

	return 0;
}
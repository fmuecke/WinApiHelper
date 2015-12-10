// Copyright 2015 Florian Muecke. All rights reserved.
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
	System::WindowsVersion winVer;
	winVer.Init();

	wcout << winVer.ToExtendedString() << endl;
    wcout << L"Default system language: " << System::GetDefaultLocaleName() << endl;
    
    std::wstring locale;
    
    Locale::LCIDToLocaleName(::GetUserDefaultUILanguage(), locale);
    wcout << L"Current user language: " << locale << endl;

	return 0;
}
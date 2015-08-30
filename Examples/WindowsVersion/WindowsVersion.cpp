#include "../../WinApiHelper/System.hpp"
#include <iostream>

using namespace std;

int main()
{
	WinApiHelper::System::WindowsVersion winVer;
	winVer.Init();

	wcout << winVer.ToExtendedString() << endl;

	return 0;
}
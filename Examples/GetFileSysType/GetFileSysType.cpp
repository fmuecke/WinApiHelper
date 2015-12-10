#include "../../WinUtil/System.hpp"
#include <iostream>

int wmain(int argc, wchar_t** argv)
{
	if (argc == 2)
	{
		//::GetVolumePathNameW(argv[1]
		//	_In_  LPCTSTR lpszFileName,
		//	_Out_ LPTSTR  lpszVolumePathName,
		//	_In_  DWORD   cchBufferLength
		//	);
		std::wcout << WinUtil::System::GetFileSystemType(argv[1]) << std::endl;
	}
	else
	{
		std::wcout << WinUtil::System::GetFileSystemType(L"") << std::endl;
	}
	
}
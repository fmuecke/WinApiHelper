#pragma once
#include <string>

namespace WinApiHelper
{
	struct UserProfile
	{
		std::wstring name;
		std::wstring sid;
		std::wstring path;
		std::wstring domain;

		std::wstring GetFullAccountName() const 
		{
			return domain.empty() ? name : domain + L"\\" + name;
		}
	};
}
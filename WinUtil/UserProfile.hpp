// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once
#include <string>

namespace WinUtil
{
	struct UserProfile
	{
        std::wstring name{};
		std::wstring sid{};
		std::wstring path{};
		std::wstring domain{};

		std::wstring GetFullAccountName() const 
		{
			return domain.empty() ? name : domain + L"\\" + name;
		}

		std::wstring GetFullAccountNameWithAt() const
		{
			return domain.empty() ? name : name + L"@" + domain;
		}

	};
}
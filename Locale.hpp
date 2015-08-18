#pragma once

#include <Windows.h>
#include <string>

namespace WinApiHelper
{
	namespace Locale
	{
		static bool LCIDToLocaleName(LCID lcid, std::wstring& localeName)
		{
			// Note: when using Windows Vista+ you can use GetSystemDefaultLocaleName()
			wchar_t buffer[LOCALE_NAME_MAX_LENGTH] = { 0 };
			if (::GetLocaleInfoW(lcid, LOCALE_SISO639LANGNAME, &buffer[0], LOCALE_NAME_MAX_LENGTH))
			{
				std::wstring langName(&buffer[0]);
				if (::GetLocaleInfoW(lcid, LOCALE_SISO3166CTRYNAME, &buffer[0], LOCALE_NAME_MAX_LENGTH))
				{
					localeName = langName + L"-" + &buffer[0];
					return true;
				}
			}
			return false;
		}
	}
}
#pragma once

#include <Windows.h>
#include <string>
#include <memory>

namespace WinApiHelper
{
	namespace Environment
	{
		static bool ExpandStrings(std::wstring const& in, std::wstring& out)
		{
			auto requiredSize = ::ExpandEnvironmentStringsW(in.c_str(), nullptr, 0);
			std::unique_ptr<wchar_t[]> pBuffer(new wchar_t[requiredSize]);
			auto bytesRead = ::ExpandEnvironmentStringsW(in.c_str(), pBuffer.get(), requiredSize);
			out = pBuffer.get();
			return bytesRead > 0 && bytesRead == requiredSize;
		}
	}

}


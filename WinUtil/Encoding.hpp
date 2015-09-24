#pragma once

#include <Windows.h>
//#include <Shlwapi.h>
//#include <WinInet.h>
#include <Wincrypt.h>
#include <string>
#include <vector>

#pragma comment(lib, "Crypt32.lib")

namespace WinUtil
{
	namespace Encoding
	{
		static std::string ToBase64(std::vector<unsigned char> const& bytes)
		{
			DWORD requiredChars = 0;
			if (::CryptBinaryToStringA(&bytes[0], bytes.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, NULL, &requiredChars))
			{
				std::vector<char> buffer(requiredChars, 0);
				if (::CryptBinaryToStringA(&bytes[0], bytes.size(), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &buffer[0], &requiredChars))
				{
					return std::string(&buffer[0]);
				}
			}

			return std::string();
		}

		static std::string ToBase64(std::string const& str)
		{
			return ToBase64(std::vector<unsigned char>(str.c_str(), str.c_str() + str.size()));
		}

		static std::vector<unsigned char> FromBase64(std::string const& base64)
		{
			std::vector<unsigned char> data(base64.size(), 0);
			DWORD nBytes = data.size();
			if (::CryptStringToBinaryA(base64.c_str(), 0, CRYPT_STRING_BASE64, data.data(), &nBytes, NULL, NULL))
			{
				data.resize(nBytes);
			}
			else
			{
				data.clear();
			}

			return data;
		}
	}
}
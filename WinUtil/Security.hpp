// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once

#include <windows.h>
#include <Sddl.h>

#include <string>
#include <memory>

namespace WinUtil
{
	namespace Security
	{
		static DWORD SidToAccountName(std::wstring const& sidStr, std::wstring& user, std::wstring& domain)
		{
			_SID* unsaveSidPointer{ nullptr };
			bool ret{ false };
			[[suppress(type.1)]] // suppress reinterpret_cast
			{
				ret = ::ConvertStringSidToSidW(sidStr.c_str(), reinterpret_cast<PSID*>(&unsaveSidPointer)) != 0;
			}
			if (!ret) return ::GetLastError();
			using DeleteFunType = HLOCAL(__stdcall*)(HLOCAL);
			auto pSid = std::unique_ptr<_SID, DeleteFunType>(unsaveSidPointer, ::LocalFree);
			SID_NAME_USE sidNameUse{};
			DWORD nameBufferSize = 0;
			DWORD domainBufferSize = 0;
			ret = ::LookupAccountSidW(nullptr, pSid.get(), nullptr, &nameBufferSize, nullptr, &domainBufferSize, &sidNameUse) != 0;
            if (!ret)
            {
                auto errorCode = ::GetLastError();
                if (errorCode != ERROR_INSUFFICIENT_BUFFER)
                {
                    if (errorCode != ERROR_NONE_MAPPED) return errorCode;
                    user.assign(sidStr);
                }
                else
                {
                    auto nameBuffer = std::vector<wchar_t>(nameBufferSize, 0);
                    auto domainBuffer = std::vector<wchar_t>(domainBufferSize, 0);
                    ret = ::LookupAccountSidW(nullptr, pSid.get(), nameBuffer.data(), &nameBufferSize, domainBuffer.data(), &domainBufferSize, &sidNameUse) != 0;
                    if (!ret) return ::GetLastError();

                    user.assign(nameBuffer.data());
                    domain.assign(domainBuffer.data());
                }
            }

			return NO_ERROR;
		}		
	}
}

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
			_SID* unsaveSidPointer;
			bool ret = ::ConvertStringSidToSidW(sidStr.c_str(), (PSID*)&unsaveSidPointer) != 0;
			if (!ret) return ::GetLastError();
			
			std::unique_ptr<_SID, HLOCAL(__stdcall *)(HLOCAL)> pSid(unsaveSidPointer, ::LocalFree);
			SID_NAME_USE sidNameUse;
			DWORD nameBufferSize = 0;
			DWORD domainBufferSize = 0;
			ret = ::LookupAccountSidW(NULL, pSid.get(), NULL, &nameBufferSize, NULL, &domainBufferSize, &sidNameUse) != 0;
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
                    std::vector<wchar_t> nameBuffer(nameBufferSize, 0);
                    std::vector<wchar_t> domainBuffer(domainBufferSize, 0);
                    ret = ::LookupAccountSidW(NULL, pSid.get(), nameBuffer.data(), &nameBufferSize, domainBuffer.data(), &domainBufferSize, &sidNameUse) != 0;
                    if (!ret) return ::GetLastError();

                    user.assign(nameBuffer.data());
                    domain.assign(domainBuffer.data());
                }
            }

			return NO_ERROR;
		}		
	}
}

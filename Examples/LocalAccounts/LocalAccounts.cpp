// Copyright 2015 Florian Muecke. All rights reserved.
#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "../../WinUtil/System.hpp"

using namespace std;

int main()
{
    try
    {
        auto accounts = WinUtil::System::GetLocalProfiles();
        for (auto const& account : accounts)
        {
            wcout << account.GetFullAccountName() << endl;
        }

        return 0;
    }
    catch (WinUtil::System::SysError const& err)
    {
        cerr << err.what();
        return err.Value();
    }
    catch (std::exception const& err)
    {
        cerr << err.what();
    }
    catch (...)
    {
        cerr << "unknown error"; 
    }

    return 1;
}
// Copyright 2015 Florian Muecke. All rights reserved.
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

using namespace std;

static bool IsMatch(string s, string pattern)
{
    string wildCardPattern;
    for (auto const c : pattern)
    {
        switch (c)
        {
            case '.': wildCardPattern.append("[.]");
                break;
            case '*': wildCardPattern.append(".*");
                break;
            case '?': wildCardPattern.append(".{1}");
                break;
            case '\\': wildCardPattern.append(R"([\\])");
                break;
            default:
                wildCardPattern.push_back(c);
        }
    }

    return regex_match(s, regex(wildCardPattern));
}

int main()
{
    bool x{ false };
    x = IsMatch(R"(c:\dev\ipponboard.exe)", R"(*\dev\*.exe)");
    x = IsMatch(R"(c:\dev\ipponboard.exe)", "*");



    return 0;
}


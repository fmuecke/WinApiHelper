// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once

#include <vector>
#include <string>
#include <algorithm>
#include <Windows.h>

namespace StdHelper
{
	template <typename T>
	std::vector<T> Split(const T& s, const T& delim, const bool keep_empty)
	{
		std::vector<T> result;
		if (delim.empty())
		{
			result.push_back(s);
			return result;
		}

		T::const_iterator substart = s.begin(), subend;
		while (true)
		{
			subend = std::search(substart, s.end(), delim.begin(), delim.end());
            T temp{ substart, subend };
			if (keep_empty || !temp.empty()) result.push_back(temp);
			if (subend == s.end())  break;
			substart = subend + delim.size();
		}

		return result;
	}

    static std::vector<std::string> SplitNoEmpties(const std::string& s, const std::string& delim)
    {
        return Split<std::string>(s, delim, false);
    }

    static std::vector<std::wstring> SplitNoEmpties(const std::wstring& s, const std::wstring& delim)
    {
        return Split<std::wstring>(s, delim, false);
    }

	static std::vector<std::string> SplitKeepEmpties(const std::string& s, const std::string& delim)
	{
		return Split<std::string>(s, delim, true);
	}

	static std::vector<std::wstring> SplitKeepEmpties(const std::wstring& s, const std::wstring& delim)
	{
		return Split<std::wstring>(s, delim, true);
	}

    static void ReplaceAll(std::string& s, std::string const& search, std::string const& replace)
    {
        for (size_t pos = 0; ; pos += replace.length())
        {
            pos = s.find(search, pos);
            if (pos == std::string::npos) break;
            s.erase(pos, search.length());
            s.insert(pos, replace);
        }
    }

    static bool InvariantCompare(const std::wstring& lhs, const std::wstring& rhs)
    {
        return ::CompareStringEx(LOCALE_NAME_INVARIANT, NORM_IGNORECASE, lhs.c_str(), lhs.size(), rhs.c_str(), rhs.size(), nullptr, nullptr, 0) == CSTR_EQUAL;
    }
}

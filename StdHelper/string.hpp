#pragma once

#include <vector>
#include <string>
#include <algorithm>

namespace StdHelper
{
	template <typename T>
	std::vector<T> split(const T& s, const T& delim, const bool keep_empty)
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
			T temp(substart, subend);
			if (keep_empty || !temp.empty()) result.push_back(temp);
			if (subend == s.end())  break;
			substart = subend + delim.size();
		}

		return result;
	}

	static std::vector<std::string> split(const std::string& s, const std::string& delim, const bool keep_empty = true)
	{
		return split<std::string>(s, delim, keep_empty);
	}

	static std::vector<std::wstring> split(const std::wstring& s, const std::wstring& delim, const bool keep_empty = true)
	{
		return split<std::wstring>(s, delim, keep_empty);
	}
}

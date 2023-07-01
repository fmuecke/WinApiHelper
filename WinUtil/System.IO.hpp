// Copyright 2015 Florian Muecke. All rights reserved.
#pragma once

#include "../StdHelper/string.hpp"
#include <string>
#include <vector>
#include <functional>
#include <filesystem>
#include <regex>

namespace WinUtil
{
    namespace System
    {
        namespace IO
        {
            // Specifies whether to search the current directory, or the current directory and
            // all subdirectories.
            enum class SearchOption
            {
                // Includes only the current directory in a search operation.
                TopDirectoryOnly = 0,

                // Includes the current directory and all its subdirectories in a search operation.
                // This option includes reparse points such as mounted drives and symbolic links
                // in the search.
                AllDirectories = 1
            };

            static bool IsPatternMatch(std::string str, std::string pattern)
            {
                if (str.empty() || pattern.empty()) return false;
                auto regexStr = std::regex_replace(pattern, std::regex("[.^$|()\\[\\]{}+\\\\]"), "\\\\&", 
                    std::regex_constants::match_default | std::regex_constants::format_sed);
                StdHelper::ReplaceAll(regexStr, "?", ".");
                StdHelper::ReplaceAll(regexStr, "*", ".*");
                return std::regex_match(str, std::regex(regexStr, std::regex_constants::icase));
            }

            static std::vector<std::string> FindFiles(std::string path, SearchOption option, std::function<bool(std::string)> matches)
            {
                namespace fs = std::filesystem;

                auto result = std::vector<std::string>();
                if (path.empty()) return result;

                if (option == SearchOption::AllDirectories)
                {
                    for (auto const& p : fs::recursive_directory_iterator(path))
                    {
                        if (!fs::is_directory(p) && matches(p.path().filename().string()))
                        {
                            result.emplace_back(std::move(p.path().string()));
                        }
                    }
                }
                else
                {
                    for (auto const& p : fs::directory_iterator(path))
                    {
                        if (!fs::is_directory(p) && matches(p.path().filename().string()))
                        {
                            result.emplace_back(std::move(p.path().string()));
                        }
                    }

                }

                return result;
            }
        }
    }
}
#include "../../WinUtil/System.hpp"
#include <iostream>
#include <filesystem>
#include <regex>
#include <array>
#include <string>
#include <iostream>
#include <iomanip>
#include <filesystem>
#include <chrono>
#include <time.h>

using namespace std;
namespace fs = std::filesystem;

// Display the last write time for the file
wstring LastWriteTimeToLocalTime(const fs::path& file_path)
{
    const auto write_time = fs::last_write_time(file_path);
    const auto write_time_point = std::chrono::time_point_cast<std::chrono::system_clock::duration>(write_time - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    const std::time_t write_time_t = std::chrono::system_clock::to_time_t(write_time_point);

    tm timeinfo;
    localtime_s(&timeinfo, &write_time_t);
    array<wchar_t, 56> buf;
    _wasctime_s(buf.data(), buf.size(), &timeinfo); // appends '\n'
    return wstring{ buf.data() };
}

// List files and directories in the specified path
void DisplayFolderContents(const fs::path& p)
{
    //wcout << L"Begin iterating " << p.wstring() << endl;
    for (const auto& entry : fs::directory_iterator{ p })
    {
        if (fs::is_regular_file(entry.status()))
        {
            wcout << L" File: " << entry.path().filename() << " : "
                << LastWriteTimeToLocalTime(entry.path());
        }
        else if (is_directory(entry.status()))
        {
            wcout << L" Dir: " << entry.path().filename() << endl;
        }
    }
}

// List files and directories recursively in the path
void IterateFolderRecursively(const fs::path& p, const string& extension)
{
    wcout << L"Begin iterating " << p.wstring() << L" recursively" << endl;
    for (fs::recursive_directory_iterator it{ p }, end; it != end; ++it)
    {
        if (fs::is_regular_file(it->status()) && fs::path(*it).extension().string() == extension)
        {
            wcout /*<< setw(it.depth())*/ << L" " << it->path() << L" : " << LastWriteTimeToLocalTime(it->path());
        }
        // else if (is_directory(it->status()))
        // {
            // wcout << setw(it.depth()) << L" " << L"Dir: " << it->path().filename() << endl;
        // }
    }
}

int wmain(int argc, wchar_t** argv)
{
	wstring dir{ LR"(C:\dev\)" };
    fs::path p{ dir };

    if (!fs::is_directory(p))
    {
        wcout << L"No such directory: " << dir << endl;
        return 1;
    }

    //DisplayFolderContents(p);
    IterateFolderRecursively(p, ".exe"); // see example 

    // wcout << endl << L"Press Enter to exit" << endl;
    // wstring input;
    // getline(wcin, input);
}
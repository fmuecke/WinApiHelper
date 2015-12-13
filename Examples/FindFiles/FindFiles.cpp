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
using namespace std::experimental::filesystem::v1;

// Display the last write time for the file
wstring LastWriteTimeToLocalTime(const path& file_path)
{
    const auto last = chrono::system_clock::to_time_t(last_write_time(file_path));
    tm timeinfo;
    localtime_s(&timeinfo, &last);
    array<wchar_t, 56> buf;
    _wasctime_s(buf.data(), buf.size(), &timeinfo); // appends '\n'
    return wstring{ buf.data() };
}

// List files and directories in the specified path
void DisplayFolderContents(const path& p)
{
    //wcout << L"Begin iterating " << p.wstring() << endl;
    for (const auto& entry : directory_iterator{ p })
    {
        if (is_regular_file(entry.status()))
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
void IterateFolderRecursively(const path& p, const string& extension)
{
    wcout << L"Begin iterating " << p.wstring() << L" recursively" << endl;
    for (recursive_directory_iterator it{ p }, end; it != end; ++it)
    {
        if (is_regular_file(it->status()) && path(*it).extension().u8string() == extension)
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
    path p{ dir };

    if (!is_directory(p))
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
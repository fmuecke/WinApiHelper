#include <iostream>
#include <memory>
#include <Windows.h>
#include <vector>
#include <string>

using namespace std;

static vector<string> GetLocicalDrives()
{
    auto size = ::GetLogicalDriveStringsA(0, nullptr);
    auto buffer = make_unique<char[]>(size);

    ::GetLogicalDriveStringsA(size, buffer.get());
    auto drives = vector<string>();
    string current;
    for (auto i = 0; i < size; ++i)
    {
        if (buffer[i])
        {
            current += buffer[i];
        }
        else
        {
            if (!current.empty())
            {
                drives.push_back(current);
                current.clear();
            }
        }
    }

    return drives;
}

enum class DriveType
{
    unknown = 0,     // the drive type cannot be determined.
    no_root_dir = 1, // the root path is invalid; for example, there is no volume mounted at the specified path.
    removable = 2,   // the drive has removable media; for example, a floppy drive, thumb drive, or flash card reader.
    fixed = 3,       // the drive has fixed media; for example, a hard disk drive or flash drive.
    remote = 4,      // the drive is a remote(network) drive.
    cdrom = 5,       // the drive is a cd - rom drive.
    ramdisk = 6      // the drive is a ram disk.
};

template <typename Enumeration>
auto as_integer(Enumeration const value) -> typename std::underlying_type<Enumeration>::type
{
    return static_cast<typename std::underlying_type<Enumeration>::type>(value);
}

std::ostream& operator << (std::ostream& os, const DriveType& t)
{
    switch (t)
    {
    default:
    case DriveType::unknown: os << "unknown"; break;
    case DriveType::cdrom: os << "cdrom"; break;
    case DriveType::fixed: os << "fixed"; break;
    case DriveType::no_root_dir: os << "no root dir"; break;
    case DriveType::ramdisk: os << "ramdisk"; break;
    case DriveType::remote: os << "remote"; break;
    case DriveType::removable: os << "removable"; break;
    }
    return os;
}

DriveType GetDriveType(string const& drive)
{
    return DriveType(::GetDriveTypeA(drive.c_str()));
}

int main()
{
    auto drives = GetLocicalDrives();
    for (auto d: drives) cout << d << " " << GetDriveType(d) << endl;
}
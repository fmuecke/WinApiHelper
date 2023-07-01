/* 
    sample program to retrieve a systems vendors product UUID as specified by
    - https://uefi.org/htmlspecs/ACPI_Spec_6_4_html/19_ASL_Reference/ACPI_Source_Language_Reference.html
    - https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.3.0.pdf
*/

#include <iostream>
#include "../../WinUtil/SystemUUID.hpp"

int main() {
    auto uuid = WinUtil::System::SystemUUID::GetAsString();
    std::cout << "the system UUID is " << uuid << std::endl << std::endl;
    std::cout << "retrieved via WMI:" << std::endl;
    system("wmic csproduct get uuid");

    return 0;
}

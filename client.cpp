

#include "libdistributed/libdistributed.hpp"

#include <iostream>

using namespace Distributed;

int main (int argc, char* argv[])
{
    std::cout << "Working...\n";
    if (argc == 1)
    {
        std::cout << "No arguments!\n";
        Node node(8888);
        node.accept();
    }
    else if (argc % 2 == 0)
    {
        std::cout << "Wrong number of arguments!\n";
    }
    else
    {
        std::cout << "Right number of arguments!\n";
    }
}

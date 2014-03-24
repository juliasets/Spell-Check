

#include "libdistributed/libdistributed.hpp"

#include <iostream>
#include <string>

using namespace Distributed;

int main (int argc, char* argv[])
{
    //Still need to add input argument functionality for jobs

    Node node(8888);

    std::cout << "Connecting...\n";
    if (argc == 1)
    {
        std::cout << "No arguments!\n";
    }
    else if (argc % 2 == 0)
    {
        std::cout << "Wrong number of arguments!\n";
    }
    else
    {
        std::cout << "Right number of arguments!\n";
        for (int i = 1; i < argc; i = i + 2)
        {
            node.join_network(argv[i],atoi(argv[i+1]));
            std::cout << "1: " + (std::string)argv[i] + ", 2: " + (std::string)argv[i+1] << std::endl;
        }
    }

    std::cout << "Starting Service: spellcheck\n";
    node.provide_service("spellcheck");

    std::cout << "Waiting for job...\n";
    node.accept();
}

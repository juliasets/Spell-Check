

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
            std::cout << "Joining network...\n";
            node.join_network(argv[i],atoi(argv[i+1]));
            std::cout << "Joined network: \n";
            std::cout << "1: " + (std::string)argv[i] + ", 2: " + (std::string)argv[i+1] << std::endl;
        }
    }

    std::cout << "Starting Service: spellcheck\n";
    node.provide_service("spellcheck");

    std::cout << "Waiting for job...\n";
    Job job = node.accept();
    std::cout << "Got job...\n";

    std::cout << "Rescinding Service: spellcheck\n";
    node.rescind_service("spellcheck");

    //Get ID from job message (delimiter should be changed later)
    job.service = job.message.substr(0, job.message.find(": "));
    std::string job_msg = job.message.substr(job.message.find(": "));

    job.message = std::string ( job_msg.rbegin(), job_msg.rend() );

    std::cout << "Sending finished job...\n";
    node.send(job);

    std::cout << "Job sent.\n";
}

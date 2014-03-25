

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
        node.join_network(argv[1],atoi(argv[2]));
    }

    //temporary example job info
    std::string job_id = "id10532";
    std::string job_msg = "This is some somple text.";
    Job job;
    job.service = "spellcheck";
    job.message = job_id + ": " + job_msg;

    std::cout << "Starting Service: unique job ID\n";
    node.provide_service(job_id);

    std::cout << "Sending job...\n";
    node.send(job);

    std::cout << "Rescinding Service: unique job ID\n";
    node.rescind_service(job_id);

    std::cout << "Final output: " + job.message << std::endl;
}


#include "../libdistributed/Slave.hpp"

#include "../libdistributed/utility.hpp"

#include "../SpellCorrector/threadedSpellCorrector.h"

#include "../libdistributed/ThreadPool.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;

using namespace SpellCorrector;

void usage ()
{
    std::cerr << "Usage: SpellCheckSlave host port" << std::endl;
}

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        usage();
        std::exit(EXIT_FAILURE);
    }
    std::string masterhost;
    unsigned short masterport;
    masterhost = argv[1];
    masterport = strtol(argv[2],NULL,0);

    Slave slave("p455w0rd");
    slave.add_master(masterhost, masterport);

    _utility::log.o << "Slave: " << slave.port() << std::endl;
    _utility::log.flush();
    
    ThreadPool tpool (4);

    _utility::log.o << "Slave: ThreadPool created." << std::endl;
    _utility::log.flush();
    
    corrector * corr = new corrector();
    
    _utility::log.o << "Slave: Corrector created." << std::endl;
    _utility::log.flush();

    corr->loadDictionary("../Dictionary/unigrams.txt");
    corr->loadErrors("../Dictionary/trained21.txt");

    _utility::log.o << "Slave: Dictionaries loaded." << std::endl;
    _utility::log.flush();

    sqlite3 *db;

    int rc;
    rc = sqlite3_open("../Dictionary/BigramDatabase.db", &db);

    _utility::log.o << "Slave: Bigram Database loaded." << std::endl;
    _utility::log.flush();


    if (rc)
    {
        _utility::log.o << "Slave: Can't open database." << std::endl;
        _utility::log.flush();
        return 1;
    }
    else
    {
        _utility::log.o << "Slave: Database opened." << std::endl;
        _utility::log.flush();
    }
    
    std::string input;
    std::string output;
    std::string first;
    std::stringstream ss;
    
    std::string result;

    for (;;)
    {
        SlaveJob job;

        if (slave.serve(job))
        {
            result = "";
            first = cmd_begin; //macro defined in threadedSpellCorrector.h
            
            ss.clear();
            ss.str(job.get_job());
            _utility::log.o << "Received job: " << ss.str() << std::endl;
            _utility::log.flush();
            while (ss >> input)
            {
                output = correct(input, corr, first, db, &tpool);
                std::stringstream ss2 (output);
                result = result + output + " ";
                while (ss2 >> first);
            }
            
            job.send_result(result);
            _utility::log.o << "Sent result: " << result <<std::endl;
            _utility::log.flush();
        }
    }
    
    tpool.shutdown();
}



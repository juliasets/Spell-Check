
#include "../libdistributed/Slave.hpp"

#include "../libdistributed/utility.hpp"

#include "../SpellCorrector/threadedSpellCorrector.h"

#include "../libdistributed/ThreadPool.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;

using namespace SpellCorrector;


int main ()
{
    unsigned short masterport;
    std::cin >> masterport;
    std::cout << masterport << std::endl; // Pass master port to next slave.

    Slave slave("p455w0rd");
    slave.add_master("127.0.0.1", masterport);

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

        _utility::log.o << "Slave: job created." << std::endl;
        _utility::log.flush();

        if (slave.serve(job))
        {
            _utility::log.o << std::endl << "Received job." << std::endl << std::endl;
            _utility::log.flush();
            
            result = "";
            first = cmd_begin; //macro defined in threadedSpellCorrector.h
            
            ss.clear();
            ss.str(job.get_job());
            _utility::log.o << "Received job: " << ss.str() << std::endl;
            while (ss >> input)
            {
		_utility::log.o << "Correcting: " << input << std::endl;
		_utility::log.flush();
                output = correct(input, corr, first, db, &tpool);
                _utility::log.o << "Slave working: "<< output << std::endl;
                _utility::log.flush();
                std::stringstream ss2 (output);
                result = result + output + " ";
                while (ss2 >> first);
            }
            
            job.send_result(result);
            _utility::log.o << "Sent job: " << result <<std::endl;
            _utility::log.flush();
        }
        _utility::log.o << "Slave serve returned" << std::endl;
        _utility::log.flush();
    }
    
    tpool.shutdown();
}



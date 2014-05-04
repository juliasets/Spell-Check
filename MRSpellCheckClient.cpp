

#include "../libdistributed/Client.hpp"

#include "../libdistributed/utility.hpp"

#include "MRmacros.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <fstream>
#include <list>
#include <time.h>

using namespace Distributed;

void usage ()
{
    std::cerr << "Usage: SpellCheckClient host port filename" << std::endl;
}

int main (int argc, char* argv[])
{
    if (argc != 4)
    {
        usage();
        std::exit(EXIT_FAILURE);
    }
    std::string masterhost;
    unsigned short masterport;
    masterhost = argv[1];
    masterport = strtol(argv[2],NULL,0);

    sleep(1);

    Client client("p455w0rd");
    client.add_master(masterhost, masterport);
    
    std::vector< std::unique_ptr<ClientJob> > jobs;
    std::vector< std::unique_ptr<ClientJob> > result_jobs;
    
    std::string filename = argv[3];
    std::ifstream in;
    in.open( filename.c_str() );
    if (!in.is_open())
    {
	_utility::log.o << "Failed to open " << filename << std::endl;
	_utility::log.flush();
	exit(1);
    }
    _utility::log.o << "Client: " << filename << std::endl;
    _utility::log.flush();

    std::string phrase;
    std::string word;
    bool end = false;
    int max_lines = 5;
    uint64_t rand_key = _utility::rand64();
    uint64_t j;
    std::vector<int> positions;
    for (j = 0; !end; ++j)
    {
        // Format: job type, key, # of lines, chunk
	std::stringstream message;
        message.clear();
        message << ACCEPT_CHUNK << "\n";
        message << filename << time(NULL) << rand_key << j << "\n";
        int chunk_lines = 0;
	
	positions.push_back(in.tellg());
	std::stringstream chunk;
        chunk.clear();
	_utility::log.o << "Chunk " << j << ":" << std::endl;
        for (int i = 0; i < max_lines && !in.eof(); ++i)
        {
            std::string line;
            getline(in, line);
	    //*
	    _utility::log.o << line << std::endl;
	    _utility::log.flush();
	    //*/
            chunk << line << "\n";
            chunk_lines = i+1;
        }
        
        message << chunk_lines << "\n";
        message << chunk.str() << "\n";
        _utility::log.o << "Message:" << std::endl;
        _utility::log.flush();
        _utility::log.o << message.str() << std::endl;
        _utility::log.flush();

        end = chunk_lines != max_lines;

        jobs.emplace_back(new ClientJob(client));
        if (!*jobs[j])
        {
            _utility::log.o << "Couldn't get slave from master." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << jobs[j]->port() << std::endl;
        _utility::log.flush();
        if (!jobs[j]->send_job(message.str()))
        {
            _utility::log.o << "Couldn't send job to slave." << std::endl;
            _utility::log.flush();
            return 1;
        }
	
    }

    in.close();

    for (uint64_t k = 0; k < j; ++k)
    {
        std::string result;

	if (jobs[k]->get_result(100000, result))
	{
	    if(!result.compare(ERROR_MESSAGE))
	    {
		while (!(result.compare(ERROR_MESSAGE))
		{
		    _utility::log.o << "ClientJob (" << jobs[k]->port() << ") failed: message not received by slave." << std::endl;
		    _utility::log.flush();          
		
		    // rewrite message
		    std::stringstream message("");
		    message << ACCEPT_CHUNK << "\n";
		    message << filename << time(NULL) << rand_key << k << "\n";
		
		    in.open(filename.c_str());
		    in.seekg(positions[k]);
		    std::stringstream chunk = ("");
		    int chunk_lines = 0;
		    for (int i = 0; i < max_lines && !in.eof(); ++i)
		    {
			std::string line;
			getline(in, line);
			chunk << line << "\n";
			chunk_lines = i+1;
		    }
		    message << chunk_lines << "\n";
		    message << chunk.str() << "\n";
		
		    jobs[k] = new ClientJob(client);
		    jobs[k]->send_job(message.str());
                    //TODO: check response from send job
		    jobs[k]->get_result(100000, result);
		}
		
	    }	
	    else if (!result.compare(RECEIVED_MESSAGE))
	    {
		//Is there a way to close jobs??
		_utility::log.o << "ClientJob (" << jobs[k]->port() << ") received." << std::endl;
		_utility::log.flush();          
	    }
	}
	else
	{
	    //Need to handle this
	    _utility::log.o << "ClientJob (" << jobs[k]->port() << ") failed: get timed out." << std::endl;
	    _utility::log.flush();      
	}
    }

    for (uint64_t l = 0; l < j; ++l)
    {
	std::stringstream message;
        message.clear();
        message << RETURN_CHUNK_RESULT << std::endl;

        result_jobs.emplace_back(new ClientJob(client));
        if (!*result_jobs[l])
        {
            _utility::log.o << "Couldn't get slave from master." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << result_jobs[l]->port() << std::endl;
        _utility::log.flush();
        if (!result_jobs[l]->send_job(message.str()))
        {
            _utility::log.o << "Couldn't receive job to slave." << std::endl;
            _utility::log.flush();
            return 1;
        }
    }
    std::string outfilename = "output.txt";
    std::ofstream out( outfilename.c_str(), std::ios_base::out);
    for (auto & job : result_jobs)
    {
	std::string result;
	if (job->get_result(100000, result))
	{

	    if(!result.compare(WAITING_MESSAGE))
	    {
                //Need to handle this
                _utility::log.o << "ClientJob (" << job->port() << ") failed." << std::endl;
	    }	
	    else
	    {
                out << result << std::endl;
                _utility::log.o << "ClientJob (" << job->port() << ") result: " << result << std::endl;
	    }
	}
	else
	{
            //Need to handle this
	    _utility::log.o << "ClientJob (" << job->port() << ") failed." << std::endl;
	}
	_utility::log.flush();      
    }
    _utility::log.o << "Success!" << std::endl;
    _utility::log.flush();
}
 
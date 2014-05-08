

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

    std::vector<std::string> messages;
    std::vector<int> positions;
    
    for (j = 0; !end; ++j)
    {
        // Format: job type, key, # of lines, chunk
	std::stringstream message;
	std::stringstream request_message;

	request_message << ACCEPT_CHUNK << "\n";
	message << filename << "-" << rand_key << time(NULL) << j << "\n";
        messages.push_back(message.str());
	int chunk_lines = 0;
	
	positions.push_back(in.tellg());
	std::stringstream chunk;
        chunk.clear();
	_utility::log.o << "Chunk " << j << ":" << std::endl;
        _utility::log.flush();
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
        request_message << message.str();
	request_message << chunk_lines << "\n";
        request_message << chunk.str() << "\n";

        _utility::log.o << "Message:" << std::endl;
        _utility::log.flush();
        _utility::log.o << request_message.str() << std::endl;
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
        
	while (!jobs[j]->send_job(request_message.str()))
        {
            _utility::log.o << "Couldn't send job to slave." << std::endl;
            _utility::log.flush();

	    jobs[j] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
        }
	
    }

    sleep(2);
    for (uint64_t k = 0; k < j; ++k)
    {
        std::string result;

	if (jobs[k]->get_result(100000, result))
	{
	    if(!result.compare(ERROR_MESSAGE))
	    {
		bool send_success = false;
		while (!(result.compare(ERROR_MESSAGE)) || !send_success)
		{
		    _utility::log.o << "ClientJob (" << jobs[k]->port() << ") failed: message not received by slave.";
		    _utility::log.flush();          
		    
		    // resend message

		    std::stringstream message;
		    message << ACCEPT_CHUNK << messages[k];
		
		    in.seekg(positions[k]);
		    std::stringstream chunk;
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

		    
		    jobs[k] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
		    jobs[k]->send_job(message.str());

		    send_success = jobs[k]->get_result(100000, result);
		    sleep(1);
		}
		
	    }	
	    else if (!result.compare(RECEIVED_MESSAGE))
	    {
		_utility::log.o << "ClientJob (" << jobs[k]->port() << ") received." << std::endl;
		_utility::log.flush();          
	    }
	}
	else
	{
	    _utility::log.o << "ClientJob (" << jobs[k]->port() << ") failed: get timed out." << std::endl;
	    _utility::log.flush();      

	    bool send_success = false;
	    while (!send_success)
	    {
		std::stringstream message;
		message << ACCEPT_CHUNK << messages[k];
		
		in.seekg(positions[k]);
		std::stringstream chunk;
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
		
		jobs[k] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
		jobs[k]->send_job(message.str());

		send_success = jobs[k]->get_result(100000, result);
	    }
	}
    }

    sleep(2);

    for (uint64_t l = 0; l < j; ++l)
    {
	std::stringstream message;
        message.clear();
        message << RETURN_CHUNK_RESULT << messages[l];
		
	in.seekg(positions[l]);
	std::stringstream chunk;
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


        result_jobs.emplace_back(new ClientJob(client, *jobs[l]));
        if (!*result_jobs[l])
        {
            _utility::log.o << "Couldn't get slave from master." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << result_jobs[l]->port() << std::endl;
        _utility::log.flush();
        while (!result_jobs[l]->send_job(message.str()))
        {
            _utility::log.o << "Couldn't receive job to slave." << std::endl;
            _utility::log.flush();
	    
	    jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
        }
    }
    std::string outfilename = "output.txt";
    std::ofstream out( outfilename.c_str(), std::ios_base::out);
//    for (auto & job : result_jobs)
    for (int l = 0; l < j; ++l)
    {

	bool receive_success = false;
	while (!receive_success)
	{
	    std::string result;
	    if (result_jobs[l]->get_result(100000, result))
	    {
		if(!result.compare(WAITING_MESSAGE))
		{
		
		    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") needs more time." << std::endl;
		    _utility::log.flush();
		
		    // Resend request for results
		    result_jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client, *result_jobs[l])));
		
		    std::stringstream message;
		    message << RETURN_CHUNK_RESULT << "\n" << messages[l];
		    
		    in.seekg(positions[l]);
		    std::stringstream chunk;
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

//-----------------------------------------------------------
//need to check that this goes through etc.
		    result_jobs[l] ->send_job(message.str());
//-----------------------------------------------------------
		
		    sleep(5);
		}
		else
		{
		    out << result;
		    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") result: " << result << std::endl;
		    _utility::log.flush();
		    receive_success = true;
		}
	    }
	    else
	    {
//-----------------------------------------------------------
//this actually does not work as desired yet, would involve a major loop over most of this file
		_utility::log.o << "A job has closed unexpectedly. Please stop and start over." << std::endl;
		_utility::log.flush();
		_utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") failed, attempting to resend." << std::endl;
		_utility::log.flush();

		// Resend job
		bool send_success = false;
		while (!send_success)
		{
		    std::stringstream message;
		    message << ACCEPT_CHUNK << "\n" << messages[l];
		    
		    in.seekg(positions[l]);
		    std::stringstream chunk;
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
		    
		    result_jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
		    result_jobs[l]->send_job(message.str());
		    
		    send_success = result_jobs[l]->get_result(100000, result);
		}
	    }
//-----------------------------------------------------------
	}
    }
    in.close();
    _utility::log.o << "Success!" << std::endl;
    _utility::log.flush();
}
 

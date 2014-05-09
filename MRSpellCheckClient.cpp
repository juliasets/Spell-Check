

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
#include <math.h>

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
    
    in.seekg (0, in.end);
    int length = in.tellg();
    if (length == 0)
    {
        _utility::log.o << filename << " is empty." << std::endl;
        _utility::log.flush();
        exit(1);
    }
    in.seekg (0, in.beg);
    
    int max_lines = ceil(((double)length)/3000);
    _utility::log.o << "Client maxlines: " << max_lines << std::endl;
    _utility::log.flush();
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
        for (int i = 0; i < max_lines && !in.eof(); ++i)
        {
            std::string line;
            getline(in, line);
            
            chunk << line << "\n";
            chunk_lines = i+1;
        }
        request_message << message.str();
        request_message << chunk_lines << "\n";
        request_message << chunk.str() << "\n";

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
        
        //check for received status
        std::string result;
        if (jobs[j]->get_result(100000, result))
        {
            if(!result.compare(ERROR_MESSAGE))
            {
                bool send_success = false;
                while (!(result.compare(ERROR_MESSAGE)) || !send_success)
                {
                    _utility::log.o << "ClientJob (" << jobs[j]->port() << ") failed: message not received by slave.";
                    _utility::log.flush();          
                    
                    // resend message
                    jobs[j] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client, *jobs[j])));
                    jobs[j]->send_job(request_message.str());

                    send_success = jobs[j]->get_result(100000, result);
                }
                _utility::log.o << "ClientJob (" << jobs[j]->port() << ") received." << std::endl;
                _utility::log.flush();          
                
            }   
            else if (!result.compare(RECEIVED_MESSAGE))
            {
                _utility::log.o << "ClientJob (" << jobs[j]->port() << ") received." << std::endl;
                _utility::log.flush();          
            }
        }
        else
        {
            _utility::log.o << "ClientJob (" << jobs[j]->port() << ") failed: slave response not received." << std::endl;
            _utility::log.flush();      

            bool send_success = false;
            while (!(result.compare(ERROR_MESSAGE)) || !send_success)
            {
                jobs[j] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
                jobs[j]->send_job(request_message.str());

                send_success = jobs[j]->get_result(100000, result);
            }
            _utility::log.o << "ClientJob (" << jobs[j]->port() << ") received." << std::endl;
            _utility::log.flush();          
                
        }
    }

    sleep(2);

    std::string outfilename = "output.txt";
    std::ofstream out( outfilename.c_str(), std::ios_base::out);

    for (uint64_t l = 0; l < j; ++l)
    {
	std::stringstream message;
	std::stringstream request_message;
        message.clear();
        request_message.clear();
		
        request_message << RETURN_CHUNK_RESULT << "\n";
        message << messages[l];
		
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

	request_message << message.str();

        result_jobs.emplace_back(new ClientJob(client, *jobs[l]));
        if (!*result_jobs[l])
        {
            _utility::log.o << "Couldn't reconnect to slave to get results." << std::endl;
            _utility::log.flush();
            return 1;
        }
        _utility::log.o << "ClientJob: " << result_jobs[l]->port() << std::endl;
        _utility::log.flush();
        while (!result_jobs[l]->send_job(request_message.str()))
        {
            _utility::log.o << "Connected to slave, but couldn't request results. Trying again..." << std::endl;
            _utility::log.flush();
	    
	    result_jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client, *result_jobs[l])));
        }

	//ask for processed result
        std::string result;
	if (result_jobs[l]->get_result(100000, result))
	{
	    if(!result.compare(WAITING_MESSAGE))
	    {
		bool send_success = false;
		while (!(result.compare(WAITING_MESSAGE)) || !send_success)
		{
		    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") busy: slave needs more time.";
		    _utility::log.flush();          
		    
		    sleep(10);          
		    
		    // resend message
		    result_jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client, *jobs[l])));
		    result_jobs[l]->send_job(request_message.str());

		    send_success = result_jobs[l]->get_result(100000, result);
		}
		out << result;
		_utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") result received: " << result << std::endl;
		_utility::log.flush();          
		
	    }	
	    else
	    {
		out << result;
		_utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") result received: " << result << std::endl;
		_utility::log.flush();          
	    }
	}
	else
	{
	    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") failed: slave response not received." << std::endl;
	    _utility::log.flush();      
	    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") failed: resubmitting job request." << std::endl;
	    _utility::log.flush();      

	    bool receive_success = false;
	    while (!(result.compare(WAITING_MESSAGE)) || !receive_success)
	    {
		std::stringstream new_request_message;
		new_request_message << "ACCEPT_CHUNK" << "\n" << message.str();
		bool send_success = false;
		while (!(result.compare(ERROR_MESSAGE)) || !send_success)
		{
		    jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client)));
		    jobs[l]->send_job(new_request_message.str());

		    send_success = jobs[l]->get_result(100000, result);
		}
		result_jobs[l] = std::move(std::unique_ptr<ClientJob>(new ClientJob(client, *jobs[l])));
		result_jobs[l]->send_job(request_message.str());

		receive_success = result_jobs[l]->get_result(100000, result);
	    }
	    out << result;
	    _utility::log.o << "ClientJob (" << result_jobs[l]->port() << ") result received." << std::endl;
	    _utility::log.flush();          
		
	}
    }
    in.close();
    _utility::log.o << "Success!" << std::endl;
    _utility::log.flush();
}
 

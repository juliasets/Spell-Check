#include "../libdistributed/Client.hpp"

#include "../libdistributed/utility.hpp"

#include "../libdistributed/Slave.hpp"

#include "../SpellCorrector/threadedSpellCorrector.h"

#include "../libdistributed/ThreadPool.hpp"

#include "MRmacros.hpp"


#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <list>
#include <time.h>
#include <stdio.h>
#include <thread>

using namespace SpellCorrector;


std::string format_word(std::string word, bool * end)
{
    std::list<std::string> words;
    for (int i = 0; i < word.length(); i++)
    {
	int begin = i;
	while (ispunct(word[begin]))
	    ++begin;
	if (begin >= word.length())
	    break;
	i = begin + 1;
	while ((!ispunct(word[i])) && (i < word.length()))
	    ++i;
	std::string tmpword = word.substr(begin, i - begin);
	words.push_back(tmpword);
	if (i < word.length())
	    (*end) = true;
    }
    std::stringstream result;
    result << words.front();
    words.pop_front();
    while (!words.empty())
    {
	result << " " << words.front();
	words.pop_front();
    }
    return result.str();
}

void performJobs(std::list<std::string> * filenames, std::mutex * queueLock)
{
    ThreadPool tpool (4);
    
    corrector * corr = new corrector();
    corr->loadDictionary("../Dictionary/unigrams.txt");
    corr->loadErrors("../Dictionary/trained21.txt");
    
    sqlite3 *db;
    int rc;
    rc = sqlite3_open("../Dictionary/BigramDatabase.db", &db);
    
    if (rc)
    {
        _utility::log.o << "Slave: Can't open database." << std::endl;
        _utility::log.flush();
    }
    else
    {
        _utility::log.o << "Slave: Database opened." << std::endl;
        _utility::log.flush();
    }
    
    std::string phrase;
    std::string word;
    bool end;
    
    std::string input;
    std::string output;
    std::string first;
    std::stringstream ss;
    
    while (true)
    {
        queueLock->lock();
        if (filenames->empty())
        {
            queueLock->unlock();
            sleep(0.05);
            continue;
        }
        
        std::string origFilename = filenames->front();
        filenames->pop_front();
        queueLock->unlock();
        
        _utility::log.o << "Slave working on: " <<origFilename << std::endl;
        _utility::log.flush();
        
        std::string workingFilename = "tmp-" + origFilename;
        std::string finishedFilename = "processed-" + origFilename;
        
        std::ifstream in( origFilename.c_str() );
        
        std::ofstream out( workingFilename.c_str() );
        
        while (in>>word)
        {
	        end = false;
	        phrase = format_word(word, &end);
	        while ((!end) && (in>>word))
            {
	            phrase += " " + format_word(word, &end);
            }
            
            ss.clear();
            ss.str(phrase);
            
            while (ss >> input)
            {
                output = correct(input, corr, first, db, &tpool);
                std::stringstream ss2 (output);
                out << output << " ";
                while (ss2 >> first);
            }
            out << std::endl;
        }
        
        rename( workingFilename.c_str(), finishedFilename.c_str());
    }
}

bool accept_chunk(std::stringstream * message, std::string key)
{
	int noLines;
	(*message) >> noLines;
	int count = 0;
	
	std::ofstream file ( key.c_str() );
	std::string line;
	
	while (std::getline((*message), line))
	{
		file << line << std::endl;
		count++;
	}
	
	if (count == noLines + 2)
		return true;
	_utility::log.o << "Slave error: Mismatched line numbers: " << count 
	    << " " << noLines << std::endl;
    _utility::log.flush();
	return false;
}

int main (int argc, char* argv[])
{
    if (argc != 3)
    {
        //usage();
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
    
    std::list<std::string> filenames;
    std::mutex queueLock;
    
    unsigned int numThreads = std::thread::hardware_concurrency();
    
    if (numThreads == 0)
        numThreads = 1;
    
    std::vector<std::thread> processors (numThreads);
    
    for (int i = 0; i < numThreads; ++i)
    {
        processors[i] = std::thread (performJobs, &filenames, &queueLock);
    }
    
    for (;;)
    {
        SlaveJob job;

        if (slave.serve(job))
        {
            std::stringstream message (job.get_job());
	        int jobType;
	        message >> jobType;
	        std::string key;
	        message >> key;
	        switch (jobType)
	        {
		        case ACCEPT_CHUNK:
			        if (!accept_chunk(&message, key))
			        {
				        job.send_result(ERROR_MESSAGE);
				        break;
			        }
			        job.send_result(RECEIVED_MESSAGE);
			        
			        queueLock.lock();
			        filenames.push_back(key);
			        queueLock.unlock();
			        break;
			        
		        case RETURN_CHUNK_RESULT:
		            std::ifstream resultFile ( ("processed-" + key).c_str() );
		            if (resultFile.is_open()) {
		                resultFile.seekg (0, resultFile.end);
                        int length = resultFile.tellg();
                        if (length == 0)
                        {
                            std::string blank = "";
                            job.send_result(blank);
                            break;
                        }
                        resultFile.seekg (0, resultFile.beg);
                        
                        char * buffer = new char [length];
                        resultFile.read (buffer,length);

                        resultFile.close();
                        
                        std::string result (buffer);
                        
                        _utility::log.o << "Slave returning result for:" 
                            << key << std::endl;
                        _utility::log.flush();
                        
                        job.send_result(result);
                        break;
		            }
		            job.send_result(WAITING_MESSAGE);
			        break;
	        }
        }
    }
}




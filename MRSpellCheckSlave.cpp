#include "../libdistributed/Client.hpp"

#include "../libdistributed/utility.hpp"

#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <list>
#include <time.h>
#include <stdio.h>

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

void performJobs(std::list<std::string> filenames, std::mutex queueLock)
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
    
    std::string output;
    std::string first;
    std::stringstream ss;
    
    while (true)
    {
        if (filenames.empty())
        {
            sleep(0.05);
            continue;
        }
        
        std::string origFilename = filenames.pop_front();
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
        
        rename( workingFilename, finishedFilename);
    }
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
    
    for (;;)
    {
        SlaveJob job;

        if (slave.serve(job))
        {
            
        }
    }
    
    
    
}




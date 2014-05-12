
//#include "../libdistributed/Slave.hpp"

#include "../libdistributed/utility.hpp"

#include "../SpellCorrector/threadedSpellCorrector.h"

#include "../libdistributed/ThreadPool.hpp"

#include <iostream>
#include <sstream>

using namespace Distributed;

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
    if (words.empty())
        return "";
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

void usage ()
{
    std::cerr << "Usage: PipeSpellCheck unigrams trained bigram_db" << std::endl;
}

int main (int argc, char* argv[])
{
    if (argc != 4)
    {
        usage();
        std::exit(EXIT_FAILURE);
    }
    
    ThreadPool tpool (1);

    _utility::log.o << "ThreadPool created." << std::endl;
    _utility::log.flush();
    
    corrector * corr = new corrector();
    
    _utility::log.o << "Corrector created." << std::endl;
    _utility::log.flush();

    corr->loadDictionary(argv[1]);
    corr->loadErrors(argv[2]);

    _utility::log.o << "Dictionaries loaded." << std::endl;
    _utility::log.flush();

    sqlite3 *db;

    int rc;
    rc = sqlite3_open(argv[3], &db);

    _utility::log.o << "Bigram Database loaded." << std::endl;
    _utility::log.flush();


    if (rc)
    {
        _utility::log.o << "Can't open database." << std::endl;
        _utility::log.flush();
        return 1;
    }
    else
    {
        _utility::log.o << "Database opened." << std::endl;
        _utility::log.flush();
    }

    std::string phrase;
    std::string word;
    bool end;

    std::string input;
    std::string output;
    std::string first;
    std::stringstream ss;
    
    for (;;)
    {
        
        while (std::cin >> word)
        {
	    end = false;
	    phrase = format_word(word, &end);
	    while ((!end) && (std::cin >> word))
            {
		phrase += " " + format_word(word, &end);
            }
            
            ss.clear();
            ss.str(phrase);
            
            while (ss >> input)
            {
                output = correct(input, corr, first, db, &tpool);
                std::stringstream ss2 (output);
		std::cout << output << " ";
                while (ss2 >> first);
            }
	    std::cout << std::endl;
        }
        
    }
    
    tpool.shutdown();
    return 0;
}

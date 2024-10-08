#ifndef _commandLineParser_hpp
#define _commandLineParser_hpp
#include <string>

using namespace std;

class commandLineParser
{
  public:
    static string input;
    static string output;
    
    commandLineParser();

    void parseCommandLine(int argc, char* argv[]);
};
#endif
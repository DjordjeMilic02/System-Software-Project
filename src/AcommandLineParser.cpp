#include <stdexcept>
#include <iostream>
#include "../inc/AcommandLineParser.hpp"

using namespace std;

string commandLineParser::input = " ";
string commandLineParser::output = " ";

commandLineParser::commandLineParser()
{
  input = "";
  output = "";
}

void commandLineParser::parseCommandLine(int argc, char* argv[])
{
  if(!(argc != 2 || argc != 4))
  {
     throw runtime_error("There must only be 2 or 4 command line arguments");
     return;
  }
  
  if(argc == 2)
  {
    input = argv[1];
    output = "out.o";
    return;
  }

  input = argv[3];
  output = argv[2];

  if ((string)argv[1] != "-o")
    {
      throw runtime_error(string("Only option is -o"));
    }
}
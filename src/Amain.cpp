#include <stdio.h>
#include <string>

#include "../inc/AcommandLineParser.hpp"
#include "../inc/AassemblyParser.hpp"

void init(int argc, char* argv[])
{
  commandLineParser cmd;
  cmd.parseCommandLine(argc,argv);
  //printf("Command line reading complete complete\n");
}

int main(int argc, char* argv[])
{
  //printf("Entry\n");
  init(argc,argv);
  
  assemblyParser parser = assemblyParser();
  parser.assemble();
  //printf("Kraj\n");
  return 0;
}
#include <iostream>
#include <string>
#include "../inc/LCommandLineParser.hpp"
#include "../inc/Linker.hpp"

int main(int argc, char* argv[])
{
    CommandLineParser parser;
    parser.parseArguments(argc, argv);

    //std::cout << "Output file: " << parser.getOutputFileName() << std::endl;

    const auto& sectionsToPlace = parser.getSectionsToPlace();
    for (const auto& sectionPair : sectionsToPlace)
    {
        //std::cout << "Section: " << sectionPair.first << " -> Address: 0x" << std::hex << sectionPair.second << std::dec << std::endl;
    }

    const auto& inputFiles = parser.getInputFiles();
    //std::cout << "Input files: ";
    for (const auto& file : inputFiles)
    {
        //std::cout << file << " ";
    }
    //std::cout << std::endl;

    Linker linker(parser);
    linker.loadObjectFiles();
    //linker.testParseInputFile(inputFiles[0]);
    linker.placeSections();
    linker.displaySectionPlacements();
    linker.defineAllSymbols();
    linker.resolveForwardReferences(linker.inputFiles, linker.globalSectionTable);
    linker.generateOutputFile(parser.getOutputFileName(), linker.globalSectionTable);

    return 0;
}
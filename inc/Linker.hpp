#ifndef LINKER_HPP
#define LINKER_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include "LsymbolTable.hpp"
#include "LrelocationTable.hpp"
#include "LsectionTable.hpp"
#include "LCommandLineParser.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include "LglobalSectionTable.hpp"

using namespace std;

class Linker {


private:
    CommandLineParser parser;
    SymbolTable globalSymbolTable;
    RelocationTable relocationTable;
    

    struct InputFile {
        std::string filename;
        SymbolTable symbolTable;
        RelocationTable relocationTable;
        SectionTable sectionTable;
    };

    std::unordered_map<std::string, uint32_t> sectionAddresses;
    std::string outputFile;
    bool hexMode;

    void parseInputFile(const std::string& filename, InputFile& inputFile);
public:
    GlobalSectionTable globalSectionTable;
    std::vector<InputFile> inputFiles;
    Linker(const CommandLineParser& parser);

    void loadObjectFiles();
    void testParseInputFile(const string& filename);
    void placeSections();
    void displaySectionPlacements() const;
    void defineAllSymbols();
    void resolveForwardReferences(std::vector<InputFile>& inputFiles, GlobalSectionTable& globalSectionTable);
    void generateOutputFile(const std::string& outputFilename, GlobalSectionTable& globalSectionTable);

};

#endif // LINKER_HPP
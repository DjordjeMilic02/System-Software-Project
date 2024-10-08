#ifndef _assemblyParser_hpp
#define _assemblyParser_hpp

#include "AcommandLineParser.hpp"
#include "AinstructionData.hpp"
#include "AassemblyLexer.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "AsymbolTable.hpp"
#include "AsectionTable.hpp"
#include "ArelocationTable.hpp"
#include "AliteralPool.hpp"

using namespace std;

class assemblyParser
{
  public:
    ifstream inputFile;
    ofstream outputFile;
    vector<string> outputs;

    SymbolTable symbolTable;
    SectionTable sectionTable;
    RelocationTable relocationTable;
    LiteralPool literalPool;

    int locationCounter;
    string currentSection;
    bool active;

    assemblyParser();

    void writeOutputTxt();
    void assemble();
    instructionData parseInstruction(string instruction);
    void executeInstruction(instructionData data);
    string detectAddressingMode(string operand);
    void resolveForwardReferences(string name);
    unsigned int resolveOperandToValue(string operand);
    int getSystemRegisterIndex(string register);
};

#endif
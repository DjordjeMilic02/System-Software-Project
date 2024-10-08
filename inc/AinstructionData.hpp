#ifndef _instructionData_hpp
#define _instructionData_hpp

#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum Type
{
  NONET,
  LABEL,
  INSTRUCTION,
  DIRECTIVE
};

enum Directives
{
  NONED,
  GLOBAL,
  EXTERN,
  SECTION,
  WORD,
  SKIP,
  END
};

enum Instructions
{
  NONEI,
  HALT,
  INT,
  IRET,
  CALL,
  RET,
  JMP,
  BEQ,
  BNE,
  BGT,
  PUSH,
  POP,
  XCHG,
  ADD,
  SUB,
  MUL,
  DIV,
  NOT,
  AND,
  OR,
  XOR,
  SHL,
  SHR,
  LD,
  ST,
  CSRRD,
  CSRWR
};

class instructionData
{
  public:
    bool valid = true;

    Type type = Type::NONET;
    Directives directive = Directives::NONED;
    Instructions instruction = Instructions::NONEI;

    string addressMode = " ";
    string r0 = " ";
    string r1 = " ";
    string r2 = " ";
    string disp = " ";

    vector<string> symbolList;
    string sectionName;
    string labelName;

    instructionData();
    void print() const;
};

#endif
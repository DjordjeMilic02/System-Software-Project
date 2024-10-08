#ifndef _assemblyLexer_hpp
#define _assemblyLexer_hpp

#include <regex>
#include <string>
using namespace std;

//LABELS

extern regex label;
extern regex labelInstruction;

//DIRECTIVES

extern regex dGlobal;
extern regex dExtern;
extern regex dSection;
extern regex dWord;
extern regex dSkip;
extern regex dEnd;

//INSTRUCTIONS

extern regex noOperandInstruction;
extern regex oneOperandSimpleInstruction;
extern regex twoOperandSimpleInstruction;
extern regex csrRd;
extern regex csrWr;

extern regex jumpsSimple;
extern regex jumpsComplex;
extern regex ldst;
extern regex ldst_ld;

//ADDRESS MODES

extern regex immed;
extern regex memdir;
extern regex regdir;
extern regex regind;
extern regex regindpom;

#endif
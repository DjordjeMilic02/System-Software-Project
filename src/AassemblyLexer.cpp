#include "../inc/AassemblyLexer.hpp"

//MISC

string decimalNumber = "-?[0-9]+";
string hexadecimalNumber = "0x[0-9A-F]+";

string symbol  = "[a-zA-Z][a-zA-Z0-9_]*";
string any = symbol + "|" + decimalNumber + "|" + hexadecimalNumber;
string any1 = "[a-zA-Z0-9_]+";

//LABELS

regex label("^\\s*([a-zA-Z_][a-zA-Z0-9_]*):\\s*$");
regex labelInstruction("^\\s*([a-zA-Z_][a-zA-Z0-9_]*):\\s*(.*)$");

//DIRECTIVES

regex dGlobal("^\\.global (" + symbol + "(\\s*,\\s*" + symbol + ")*)$");
regex dExtern("^\\.extern (" + symbol + "(\\s*,\\s*" + symbol + ")*)$");
regex dSection("^\\.section (" + symbol + ")$");
regex dWord("^\\.word\\s+(" + any1 + "(\\s*,\\s*" + any1 + ")*)$");
regex dSkip("^\\.skip (" + any + "|" + any + ")$");
regex dEnd("^\\.end$");

//INSTRUCTIONS

regex noOperandInstruction("^(halt|int|iret|ret)$");
regex oneOperandSimpleInstruction("^(push|pop|not) (%r[0-9]|%r1[0-5]|%sp|%pc)$");
regex twoOperandSimpleInstruction("^(xchg|add|sub|mul|div|and|or|xor|shl|shr)\\s+(%r[0-9]|%r1[0-5]|%sp|%pc)\\s*,\\s*(%r[0-9]|%r1[0-5]|%sp|%pc)$");
regex csrRd("^(csrrd) (%status|%handler|%cause), (%r1[0-5]|%r[0-9]|%sp|%pc)");
regex csrWr("^(csrwr) (%r[0-9]|%r1[0-5]|%sp|%pc),\\s*(%status|%handler|%cause)$");

regex jumpsSimple("^(call|jmp) (.*)$");
regex jumpsComplex("^(beq|bne|bgt)\\s+(%r[0-9]|%r1[0-5]|%sp|%pc)\\s*,\\s+(%r[0-9]|%r1[0-5]|%sp|%pc)\\s*,\\s*(.*)$");
regex ldst("^(st)\\s+(%r[0-9]|%r1[0-5]|%sp|%pc),\\s*(.*)$");
regex ldst_ld("^(ld)\\s*(.*),\\s*(%r[0-9]|%r1[0-5]|%sp|%pc)$");

//ADDRESS MODES

regex immed("^\\$(" + any + ")$");
regex memdir("^(" + any + ")$");
regex regdir("^%(r[0-9]|%r1[0-5]|sp|pc)$");
regex regind("^\\[\\s*(%r[0-9]|%r1[0-5]|%sp|%pc)\\s*\\]$");
regex regindpom("^\\[\\s*(%r[0-9]|%r1[0-5]|%sp|%pc)\\s*\\+\\s*(" + any + ")\\s*\\]$");
#include "../inc/AinstructionData.hpp"

using namespace std;

instructionData::instructionData()
{
    valid = true;
    type = Type::NONET;
    directive = Directives::NONED;
    instruction = Instructions::NONEI;
    addressMode = "";
    r0 = "";
    r1 = "";
    r2 = "";
    disp = "";
    labelName = "";
}

void instructionData::print() const
{
    cout << "Instruction Data:" << endl;
    cout << "Valid: " << (valid ? "true" : "false") << endl;
    
    cout << "Type: ";
    switch (type)
    {
        case Type::NONET:       cout << "None"; break;
        case Type::LABEL:       cout << "Label"; break;
        case Type::INSTRUCTION: cout << "Instruction"; break;
        case Type::DIRECTIVE:   cout << "Directive"; break;
    }
    cout << endl;

    cout << "Directive: ";
    switch (directive)
    {
        case Directives::NONED:   cout << "None"; break;
        case Directives::GLOBAL:  cout << "Global"; break;
        case Directives::EXTERN:  cout << "Extern"; break;
        case Directives::SECTION: cout << "Section"; break;
        case Directives::WORD:    cout << "Word"; break;
        case Directives::SKIP:    cout << "Skip"; break;
        case Directives::END:     cout << "End"; break;
    }
    cout << endl;

    cout << "Instruction: ";
    switch (instruction)
    {
        case Instructions::NONEI: cout << "None"; break;
        case Instructions::HALT:  cout << "Halt"; break;
        case Instructions::INT:   cout << "Int"; break;
        case Instructions::IRET:  cout << "Iret"; break;
        case Instructions::CALL:  cout << "Call"; break;
        case Instructions::RET:   cout << "Ret"; break;
        case Instructions::JMP:   cout << "Jmp"; break;
        case Instructions::BEQ:   cout << "Beq"; break;
        case Instructions::BNE:   cout << "Bne"; break;
        case Instructions::BGT:   cout << "Bgt"; break;
        case Instructions::PUSH:  cout << "Push"; break;
        case Instructions::POP:   cout << "Pop"; break;
        case Instructions::XCHG:  cout << "Xchg"; break;
        case Instructions::ADD:   cout << "Add"; break;
        case Instructions::SUB:   cout << "Sub"; break;
        case Instructions::MUL:   cout << "Mul"; break;
        case Instructions::DIV:   cout << "Div"; break;
        case Instructions::NOT:   cout << "Not"; break;
        case Instructions::AND:   cout << "And"; break;
        case Instructions::OR:    cout << "Or"; break;
        case Instructions::XOR:   cout << "Xor"; break;
        case Instructions::SHL:   cout << "Shl"; break;
        case Instructions::SHR:   cout << "Shr"; break;
        case Instructions::LD:    cout << "Ld"; break;
        case Instructions::ST:    cout << "St"; break;
        case Instructions::CSRRD: cout << "Csrrd"; break;
        case Instructions::CSRWR: cout << "Csrwr"; break;
    }
    cout << endl;

    cout << "Address Mode: " << addressMode << endl;
    cout << "r0: " << r0 << endl;
    cout << "r1: " << r1 << endl;
    cout << "r2: " << r2 << endl;
    cout << "Displacement: " << disp << endl;

    if (!symbolList.empty())
    {
        cout << "Symbols: ";
        for (const auto &symbol : symbolList)
        {
            cout << symbol << " ";
        }
        cout << endl;
    }

    if (!sectionName.empty())
    {
        cout << "Section: " << sectionName << endl;
    }

    if(!labelName.empty())
    {
        cout << "Label:" << labelName << endl;
    }

    cout << "\n";
}
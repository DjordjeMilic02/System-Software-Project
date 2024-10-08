#include "../inc/AassemblyParser.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

assemblyParser::assemblyParser()
{
  locationCounter = 0;
  currentSection = "";
  active = true;
}

void assemblyParser::assemble()
{
  inputFile.open(commandLineParser::input);
  outputFile.open(commandLineParser::output);

  string line;
  while(getline(inputFile,line) && active)
  {
      line = regex_replace(line, regex("^\\s+|\\s+$"), "");
      size_t commentPos = line.find('#');
      if (commentPos != string::npos)
      {
            line = line.substr(0, commentPos);
      }
      line = regex_replace(line, regex("^\\s+|\\s+$"), "");
      if (line.empty() || line[0] == '#')
      {
        continue;
      }
      instructionData data = parseInstruction(line);
      //data.print();
      executeInstruction(data);
  }

  writeOutputTxt();

  inputFile.close();
  outputFile.close();
}

instructionData assemblyParser::parseInstruction(string instruction)
{
    instructionData data;
    smatch parts;

    //LABELS
    if (regex_match(instruction, parts, label))
    {
        data.type = Type::LABEL;
        data.labelName = parts.str(1);
        data.valid = true;
        return data;
    }

    if (regex_match(instruction, parts, labelInstruction))
    {
        data.labelName = parts.str(1);
        instruction = parts.str(2);
    }

    //DIRECTIVES
    if (regex_search(instruction, parts, dGlobal))
    {
        data.directive = GLOBAL;
        data.type = DIRECTIVE;
        string symbols = parts.str(1);
        stringstream ss(symbols);
        string symbol;
        while (getline(ss, symbol, ','))
        {
            symbol.erase(0, symbol.find_first_not_of(" \t"));
            symbol.erase(symbol.find_last_not_of(" \t") + 1);
            data.symbolList.push_back(symbol);
        }
        return data;
    }

    if (regex_search(instruction, parts, dExtern))
    {
        data.type = Type::DIRECTIVE;
        data.directive = Directives::EXTERN;
        string symbols = parts.str(1);
        stringstream ss(symbols);
        string symbol;
        while (getline(ss, symbol, ','))
        {
            symbol.erase(0, symbol.find_first_not_of(" \t"));
            symbol.erase(symbol.find_last_not_of(" \t") + 1);
            data.symbolList.push_back(symbol);
        }
        return data;
    }

    if (regex_search(instruction, parts, dSection))
    {
        data.directive = SECTION;
        data.type = DIRECTIVE;
        data.sectionName = parts.str(1);
        return data;
    }

    if (regex_search(instruction, parts, dWord))
    {
        data.type = Type::DIRECTIVE;
        data.directive = Directives::WORD;
        string symbols = parts.str(1);
        stringstream ss(symbols);
        string symbol;
        while (getline(ss, symbol, ','))
        {
            data.symbolList.push_back(symbol);
        }
        return data;
    }

    if (regex_search(instruction, parts, dSkip))
    {
        data.type = Type::DIRECTIVE;
        data.directive = Directives::SKIP;
        data.r0 = parts.str(1);
        return data;
    }

    if (regex_search(instruction, parts, dEnd))
    {
        data.type = Type::DIRECTIVE;
        data.directive = Directives::END;
        return data;
    }

    //INSTRUCTIONS
    if (regex_search(instruction, parts, jumpsSimple))
    {
        data.type = Type::INSTRUCTION;
        string instructionType = parts.str(1);
    
        if (instructionType == "call")
        {
            data.instruction = Instructions::CALL;
        }
        else if (instructionType == "jmp")
        {
            data.instruction = Instructions::JMP;
        }
        string p2 = parts.str(2);
        data.r0 = parts.str(2);
        data.disp = parts.str(2);
        data.addressMode = detectAddressingMode(p2);
        smatch operand;
        if(regex_search(p2, operand, regindpom))
        {
          data.r0 = operand.str(1);
          data.disp = operand.str(2);
        }
        return data;
    }

    if (regex_search(instruction, parts, jumpsComplex))
    {
        data.type = Type::INSTRUCTION;
        
        string mnemonic = parts.str(1);

        if (mnemonic == "beq") {
            data.instruction = Instructions::BEQ;
        } else if (mnemonic == "bne") {
            data.instruction = Instructions::BNE;
        } else if (mnemonic == "bgt") {
            data.instruction = Instructions::BGT;
        }

        string p2 = parts.str(2);
        data.r0 = parts.str(2);
        data.r1 = parts.str(3);
        data.disp = parts.str(4);

        data.addressMode = detectAddressingMode(data.disp);

        smatch operand;
        if (regex_search(data.r2, operand, regindpom))
        {
            data.r2 = operand.str(1);
            data.disp = operand.str(2);
        }

        return data;
    }

    if (regex_search(instruction, parts, ldst))
    {
        data.type = Type::INSTRUCTION;

        data.instruction = Instructions::ST;

        data.r0 = parts.str(2);

        string operand = parts.str(3);
        data.addressMode = detectAddressingMode(operand);

        smatch operandMatch;
        if (regex_search(operand, operandMatch, regindpom))
        {
            data.r1 = operandMatch.str(1);
            data.disp = operandMatch.str(2);
        }
        else
        {
            data.r1 = operand;
        }

        return data;
    }

    if (regex_search(instruction, parts, ldst_ld))
    {
        data.type = Type::INSTRUCTION;

        data.instruction = Instructions::LD;

        data.r0 = parts.str(3);
        string operand = parts.str(2);
        data.addressMode = detectAddressingMode(operand);

        smatch operandMatch;
        if (regex_search(operand, operandMatch, regindpom))
        {
            data.r1 = operandMatch.str(1);
            data.disp = operandMatch.str(2);
        }
        else
        {
            data.r1 = operand;
        }

        return data;
    }

    if (regex_search(instruction, parts, noOperandInstruction))
    {
        data.type = Type::INSTRUCTION;

        string instr = parts.str(1);

        if (instr == "halt")
        {
            data.instruction = Instructions::HALT;
        }
        else if (instr == "int")
        {
            data.instruction = Instructions::INT;
        }
        else if (instr == "iret")
        {
            data.instruction = Instructions::IRET;
        }
        else if (instr == "ret")
        {
            data.instruction = Instructions::RET;
        }

        return data;
    }

    if (regex_search(instruction, parts, oneOperandSimpleInstruction))
    {
        data.type = Type::INSTRUCTION;

        string instr = parts.str(1);

        if (instr == "push")
        {
            data.instruction = Instructions::PUSH;
        }
        else if (instr == "pop")
        {
            data.instruction = Instructions::POP;
        }
        else if (instr == "not")
        {
            data.instruction = Instructions::NOT;
        }

        data.r0 = parts.str(2);

        return data;
    }

    if (regex_search(instruction, parts, twoOperandSimpleInstruction))
    {
        data.type = Type::INSTRUCTION;

        string instr = parts.str(1);

        if (instr == "xchg")
        {
            data.instruction = Instructions::XCHG;
        }
        else if (instr == "add")
        {
            data.instruction = Instructions::ADD;
        }
        else if (instr == "sub")
        {
            data.instruction = Instructions::SUB;
        }
        else if (instr == "mul")
        {
            data.instruction = Instructions::MUL;
        }
        else if (instr == "div")
        {
            data.instruction = Instructions::DIV;
        }
        else if (instr == "and")
        {
            data.instruction = Instructions::AND;
        }
        else if (instr == "or")
        {
            data.instruction = Instructions::OR;
        }
        else if (instr == "xor")
        {
            data.instruction = Instructions::XOR;
        }
        else if (instr == "shl")
        {
            data.instruction = Instructions::SHL;
        }
        else if (instr == "shr")
        {
            data.instruction = Instructions::SHR;
        }

        data.r0 = parts.str(2);
        data.r1 = parts.str(3);

        return data;
    }

    if (regex_search(instruction, parts, csrRd))
    {
        data.type = Type::INSTRUCTION;
        data.instruction = Instructions::CSRRD;
        data.r0 = parts.str(2);
        data.r1 = parts.str(3);
        return data;
    }

    if (regex_search(instruction, parts, csrWr))
    {
        data.type = Type::INSTRUCTION;
        data.instruction = Instructions::CSRWR;
        data.r0 = parts.str(2);
        data.r1 = parts.str(3);
        return data;
    }

    data.valid = false;
    if(data.valid == false)
    {
        throw runtime_error("INVALID INSTRUCTION");
    }
    return data;
}

string assemblyParser::detectAddressingMode(string operand)
{
    if (std::regex_match(operand, immed))
    {
        return "immed";
    }
    else if (std::regex_match(operand, memdir))
    {
        return "memdir";
    }
    else if (std::regex_match(operand, regdir))
    {
        return "regdir";
    }
    else if (std::regex_match(operand, regind))
    {
        return "regind";
    }
    else if (std::regex_match(operand, regindpom))
    {
        return "regindpom";
    }
    else
    {
        return "unknown";
    }
}

int getRegisterIndex(const string& reg)
{
    if (reg == "%sp") return 14;
    if (reg == "%pc") return 15;
    if (reg.length() > 2 && reg.substr(0, 2) == "%r")
    {
        int regNum = stoi(reg.substr(2));
        if (regNum >= 0 && regNum <= 15)
        {
            return regNum;
        }
    }
    throw runtime_error("Invalid register: " + reg);
}

void assemblyParser::executeInstruction(instructionData data)
{
        if (data.type == Type::DIRECTIVE)
        {
            switch (data.directive)
            {
                case Directives::GLOBAL:
                    for (const string& symbolName : data.symbolList)
                    {
                        if (symbolTable.isSymbolDefined(symbolName))
                            {
                                if (!symbolTable.isGlobalSymbol(symbolName))
                                {
                                    symbolTable.setGlobalSymbol(symbolName);
                                }
                            }
                            else
                            {
                                symbolTable.addSymbol(symbolName, currentSection, 0, SymbolType::LABEL, true, 0);
                            }
                    }
                    break;
                case Directives::EXTERN:
                    for (const string& symbolName : data.symbolList)
                    {
                        symbolTable.addSymbol(symbolName, currentSection, 0, SymbolType::EXTERNAL, false, 0);
                        symbolTable.setExternalSymbol(symbolName, "NONE");
                    }
                    break;
                case Directives::SECTION:
                    if (sectionTable.sectionExists(data.sectionName))
                    {
                        throw std::runtime_error("Error: Section '" + data.sectionName + "' already exists.");
                    }

                    sectionTable.addSection(data.sectionName);
                    currentSection = data.sectionName;
                    symbolTable.addSymbol(currentSection, currentSection, locationCounter, SymbolType::SECTION, false, 0);
                    symbolTable.defineSymbol(currentSection, currentSection, 0);
                    locationCounter = 0;

                    break;
                case Directives::WORD:
                {
                    for (const auto& symbol : data.symbolList)
                    {
                        bool isNumeric = all_of(symbol.begin(), symbol.end(), ::isdigit);
                        bool isHexadecimal = (symbol.size() > 2 && (symbol.substr(0, 2) == "0x" || symbol.substr(0, 2) == "0X"));

                        if (isNumeric || isHexadecimal)
                        {
                            int value = isHexadecimal ? std::stoi(symbol, nullptr, 16) : std::stoi(symbol);
                            vector<uint8_t> data(4, 0);

                            for (int i = 3; i >= 0; --i)
                            {
                                data[i] = (value >> (8 * i)) & 0xFF;
                            }

                            sectionTable.writeDataToSection(currentSection, data);
                            locationCounter += 4;
                        }
                        else
                        {
                            RelocationEntry relocation;
                            relocation.offset = locationCounter;
                            relocation.type = RelocationType::ABSOLUTE;
                            relocation.symbolName = symbol;
                            relocation.addend = 0;
                            relocation.isInstruction = false;

                            relocationTable.addRelocation(currentSection, relocation);

                            vector<uint8_t> data(4, 0);
                            sectionTable.writeDataToSection(currentSection, data);

                            locationCounter += 4;
                        }
                    }
                    break;
                }
                case Directives::SKIP:
                {

                    const string& literal = data.r0;
                    bool isNumeric = all_of(literal.begin(), literal.end(), ::isdigit);

                    if (!isNumeric)
                    {
                        throw runtime_error(".skip ocekuje broj");
                    }

                    int skipSize = stoi(literal);

                    if (skipSize < 0)
                    {
                        throw runtime_error(".skip ocekuje pozitivan broj.");
                    }

                    vector<uint8_t> data(skipSize, 0);

                    sectionTable.writeDataToSection(currentSection, data);
                    locationCounter += skipSize;

                    break;
                }
                case Directives::END:
                {
                    active = false;
                    break;
                }
                default:
                    break;
            }
        }
        if (data.type == Type::LABEL)
        {
            const string& label = data.labelName;
            
            if (symbolTable.isSymbolDefined(label))
            {
                throw runtime_error("Error: Label '" + label + "' is already defined.");
            }
            symbolTable.addSymbol(label, currentSection, locationCounter, SymbolType::LOCAL, 0, 0);
            symbolTable.defineSymbol(label, currentSection, locationCounter);
        }
        else if (data.type == Type::INSTRUCTION && !data.labelName.empty())
        {
            const string& label = data.labelName;

            if (symbolTable.isSymbolDefined(label)) {
                throw runtime_error("Error: Label '" + label + "' is already defined.");
            }
            symbolTable.addSymbol(label, currentSection, locationCounter, SymbolType::LOCAL, 0, 0);
            symbolTable.defineSymbol(label, currentSection, locationCounter);
        }
    if(data.type == Type::INSTRUCTION)
        switch(data.instruction)
        {
            case(Instructions::HALT):
            {
                vector<uint8_t> data(4, 0);

                sectionTable.writeDataToSection(currentSection, data);
                locationCounter += 4;
                break;
            }
            case Instructions::INT:
            {
                vector<uint8_t> instructionCode = {0x10, 0x00, 0x00, 0x00};

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;
                break;
            }
            case Instructions::ADD:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x50; 
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = (srcRegister << 4) & 0xF0;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::SUB:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);
                instructionCode[0] = 0x51;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);

                instructionCode[2] = (srcRegister << 4) & 0xF0;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);

                locationCounter += 4;
                break;
            }
            case Instructions::MUL:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x52;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = (srcRegister << 4) & 0xF0;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::DIV:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x53;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = (srcRegister << 4) & 0xF0;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::NOT:
            {
                uint8_t regIndex = getRegisterIndex(data.r0);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x60;
                instructionCode[1] = (regIndex << 4 & 0xF0) | (regIndex & 0x0F);

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::AND:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);
                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x61;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = srcRegister << 4;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::OR:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x62;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = srcRegister << 4;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::XOR:
            {
                uint8_t srcRegister = getRegisterIndex(data.r0);
                uint8_t dstRegister = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x63;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = srcRegister << 4;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::SHL:
            {
                uint8_t srcRegister = getRegisterIndex(data.r1);
                uint8_t dstRegister = getRegisterIndex(data.r0);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x70;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = srcRegister << 4;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);

                locationCounter += 4;

                break;
            }
            case Instructions::SHR:
            {
                uint8_t srcRegister = getRegisterIndex(data.r1);
                uint8_t dstRegister = getRegisterIndex(data.r0);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x71;
                instructionCode[1] = (dstRegister << 4 & 0xF0) | (dstRegister & 0x0F);
                instructionCode[2] = srcRegister << 4;
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::XCHG:
            {
                uint8_t reg1 = getRegisterIndex(data.r0);
                uint8_t reg2 = getRegisterIndex(data.r1);

                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x40;
                instructionCode[1] = 0x00 | reg1;
                instructionCode[2] = 0x00 | (reg2 << 4);
                instructionCode[3] = 0x00;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::LD:
            {
                if (data.addressMode == "immed" || data.addressMode == "memdir")
                {
                    vector<uint8_t> instructionCode(4, 0);

                    instructionCode[0] = ((data.addressMode == "immed") ? 0x91 : 0x92);
                    instructionCode[1] = ((getRegisterIndex(data.r0) << 4) & 0xF0);
                    unsigned int dispValue = resolveOperandToValue(data.r1);
                    instructionCode[2] = (dispValue >> 8) & 0x0F;
                    instructionCode[3] = (dispValue) & 0xFF;

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;
                }
                else if (data.addressMode == "regdir")
                {
                    vector<uint8_t> instructionCode(4, 0);

                    instructionCode[0] = 0x91;
                    instructionCode[1] = ((getRegisterIndex(data.r0) & 0x0F) << 4) | (getRegisterIndex(data.r1) & 0x0F);
                    instructionCode[3] = 0x00;
                    instructionCode[3] = 0x00;

                    sectionTable.writeDataToSection(currentSection, instructionCode);

                    locationCounter += 4;
                }
                else if (data.addressMode == "regind" || data.addressMode == "regindpom")
                {
                    vector<uint8_t> instructionCode(4);

                    instructionCode[0] = 0x92;
                    string e = data.r1;
                    if(data.addressMode == "regind")
                    {
                        e = data.r1.erase(0, 1);
                        e.erase(e.size() - 1, 1);
                    }
                    instructionCode[1] = (getRegisterIndex(data.r0) << 4) | getRegisterIndex(e);

                    if (data.addressMode == "regindpom")
                    {
                        unsigned int dispValue = resolveOperandToValue(data.disp);
                        instructionCode[2] = ((dispValue >> 8) & 0x0F);
                        instructionCode[3] = (dispValue & 0xFF);
                    }
                    else
                    {
                        instructionCode[2] = 0x00;
                        instructionCode[3] = 0x00;
                    }

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;
                }
                else
                {
                    throw runtime_error("Nepostojeci nacin adresiranja za LD instrukciju.");
                }

                break;
            }
            case Instructions::ST:
            {
                vector<uint8_t> instructionCode(4, 0x00);

                instructionCode[0] = 0x80;

                if (data.addressMode == "immed" || data.addressMode == "regdir")
                {
                    throw runtime_error("Invalid addressing mode for store instruction.");
                }

                if (data.addressMode == "memdir")
                {
                    instructionCode[1] = 0x00;
                    instructionCode[2] = getRegisterIndex(data.r0) << 4;

                    uint16_t displacement = resolveOperandToValue(data.r1);

                    instructionCode[2] |= (displacement >> 8) & 0x0F;
                    instructionCode[3] = displacement & 0xFF;

                } else if (data.addressMode == "regind")
                {
                    string e;
                    if(data.addressMode == "regind")
                    {
                        e = data.r1.erase(0, 1);
                        e.erase(e.size() - 1, 1);
                    }
                    instructionCode[1] = getRegisterIndex(e) << 4;
                    instructionCode[2] = getRegisterIndex(data.r0) << 4;

                } else if (data.addressMode == "regindpom") {
                    instructionCode[1] = getRegisterIndex(data.r1) << 4;
                    instructionCode[2] = getRegisterIndex(data.r0) << 4;

                    uint16_t displacement = resolveOperandToValue(data.disp);

                    instructionCode[2] |= (displacement >> 8) & 0x0F;
                    instructionCode[3] = displacement & 0xFF;

                } else {
                    throw runtime_error("Los nacin adresiranja za ST instruciju.");
                }

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::CSRRD:
            {
                vector<uint8_t> instructionCode(4, 0x00);

                instructionCode[0] = 0x90;

                int gprIndex = getRegisterIndex(data.r1);
                int csrIndex = getSystemRegisterIndex(data.r0);

                instructionCode[1] = (gprIndex << 4) | csrIndex;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }

            case Instructions::CSRWR:
            {
                vector<uint8_t> instructionCode(4, 0x00);

                instructionCode[0] = 0x94;

                int csrIndex = getSystemRegisterIndex(data.r1);
                int gprIndex = getRegisterIndex(data.r0);

                instructionCode[1] = (csrIndex << 4) | gprIndex;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }

            case Instructions::CALL:
            {
                vector<uint8_t> instructionCode(4, 0);

                instructionCode[0] = 0x20;
                instructionCode[1] = 0x00;

                int value = resolveOperandToValue(data.disp);

                instructionCode[2] = (value >> 8) & 0x0F;
                instructionCode[3] = value & 0xFF;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;
                break;
            }
            case Instructions::JMP:
            {
                if (data.addressMode == "memdir")
                {
                    vector<uint8_t> instructionCode(4, 0);

                    int value = resolveOperandToValue(data.disp);

                    instructionCode.resize(4, 0);

                    instructionCode[0] = 0x30;
                    instructionCode[1] = 0x00;
                    instructionCode[2] = (value >> 8) & 0x0F;
                    instructionCode[3] = value & 0xFF;

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;

                    break;
                } else
                {
                    throw runtime_error("JMP moze samo da memdirom.");
                }
            }
            case Instructions::BEQ:
            {
                if (data.addressMode == "memdir")
                {
                    int value = resolveOperandToValue(data.disp);

                    vector<uint8_t> instructionCode(4, 0);

                    instructionCode[0] = 0x31;
                    instructionCode[1] = 0x00 | (getRegisterIndex(data.r0) & 0x0F);
                    instructionCode[2] = (getRegisterIndex(data.r1) << 4 & 0xF0) | (value >> 8) & 0x0F;
                    instructionCode[3] = value & 0xFF;

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;

                    break;
                } else
                {
                    throw runtime_error("BEQ moze samo sa memdir.");
                }
            }

            case Instructions::BNE:
            {
                if (data.addressMode == "memdir")
                {
                    int value = resolveOperandToValue(data.disp);

                    vector<uint8_t> instructionCode(4, 0);

                    instructionCode[0] = 0x32;
                    instructionCode[1] = 0x00 | (getRegisterIndex(data.r0) & 0x0F);
                    instructionCode[2] = (getRegisterIndex(data.r1) << 4 & 0xF0) | (value >> 8) & 0x0F;
                    instructionCode[3] = value & 0xFF;

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;

                    break;
                } else
                {
                    throw runtime_error("BNE moze samo sa memdir.");
                }
            }

            case Instructions::BGT: 
            {
                if (data.addressMode == "memdir")
                {
                    int value = resolveOperandToValue(data.disp);

                    vector<uint8_t> instructionCode(4, 0);

                    instructionCode[0] = 0x33;
                    instructionCode[1] = 0x00 | (getRegisterIndex(data.r0) & 0x0F);
                    instructionCode[2] = (getRegisterIndex(data.r1) << 4 & 0xF0) | (value >> 8) & 0x0F;
                    instructionCode[3] = value & 0xFF;

                    sectionTable.writeDataToSection(currentSection, instructionCode);
                    locationCounter += 4;

                    break;
                } else
                {
                    throw runtime_error("BGT moze samo sa memdir.");
                }
            }
            case Instructions::PUSH:
            {
                vector<uint8_t> instructionCode(4, 0);
                int value = literalPool.addLiteral(currentSection, "-4", locationCounter);

                instructionCode[0] = 0x81;
                instructionCode[1] = 0xE0;
                instructionCode[2] = (getRegisterIndex(data.r0) << 4 & 0xF0) | (value >> 8) & 0x0F;
                instructionCode[3] = value & 0xFF;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::POP:
            {
                vector<uint8_t> instructionCode(4, 0);
                int value = literalPool.addLiteral(currentSection, "4", locationCounter);

                instructionCode[0] = 0x93;
                instructionCode[1] = ((getRegisterIndex(data.r0) & 0xF) << 4) | 0xE;
                instructionCode[2] = (value >> 8) & 0x0F;
                instructionCode[3] = value & 0xFF;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::RET:
            {
                vector<uint8_t> instructionCode(4, 0);
                
                uint32_t value = literalPool.addLiteral(currentSection, "4", locationCounter);

                instructionCode[0] = 0x93;
                instructionCode[1] = 0xFE;
                instructionCode[2] = (value >> 8) & 0x0F;
                instructionCode[3] = value & 0xFF;

                sectionTable.writeDataToSection(currentSection, instructionCode);
                locationCounter += 4;

                break;
            }
            case Instructions::IRET:
            {
                vector<uint8_t> machineCode(8, 0);

                uint32_t disp4Address = literalPool.addLiteral(currentSection, "4", locationCounter);
                machineCode[0] = 0x97;
                machineCode[1] = 0x0E;
                machineCode[2] = (disp4Address >> 8) & 0x0F;
                machineCode[3] =  disp4Address & 0xFF;

                locationCounter += 4;

                uint32_t disp8Address = literalPool.addLiteral(currentSection, "4", locationCounter);
                machineCode[4] = 0x93;
                machineCode[5] = 0xFE;
                machineCode[6] = (disp8Address >> 8) & 0x0F;
                machineCode[7] =  disp8Address & 0xFF;

                sectionTable.writeDataToSection(currentSection, machineCode);
                locationCounter += 4;

                break;
            }
    
        default:
        break;
    }
 
    return;
}

int assemblyParser::getSystemRegisterIndex(string registerName)
{
    string regName = registerName;
    
    if (!regName.empty() && regName[0] == '%')
    {
        regName.erase(0, 1);
    }

    if (regName == "status")
    {
        return 0;
    } else if (regName == "handler")
    {
        return 1;
    } else if (regName == "cause")
    {
        return 2;
    } else
    {
        throw runtime_error("Unknown system register: " + registerName);
    }
}

unsigned int assemblyParser::resolveOperandToValue(string operand1)
{
    string operand;
    if (!operand1.empty() && operand1[0] == '$')
    {
        operand = operand1.substr(1);
    } else
    {
        operand = operand1;
    }

    if (operand.size() > 2 && (operand.substr(0, 2) == "0x" || operand.substr(0, 2) == "0X"))
    {
        //cout << operand << endl;
        unsigned int value = stoul(operand, nullptr, 16);
        string v = to_string(value);
        unsigned int literalAddress = literalPool.addLiteral(currentSection, v, locationCounter);

        return literalAddress;
    }

    if (all_of(operand.begin(), operand.end(), ::isdigit))
    {
        int value = stoi(operand);

        string v = to_string(value);
        unsigned int literalAddress = literalPool.addLiteral(currentSection, v, locationCounter);

        return literalAddress;
    } else
    {
        if (symbolTable.isSymbolDefined(operand))
        {
            unsigned int symbolAddress = symbolTable.getSymbolAddress(operand);
            
            string symbolAddressStr = to_string(symbolAddress);
            unsigned int literalAddress = literalPool.addLiteral(currentSection, symbolAddressStr, locationCounter);

            RelocationEntry relocation;
            relocation.offset = locationCounter;
            relocation.type = RelocationType::ABSOLUTE;
            relocation.symbolName = operand;
            relocation.addend = literalAddress - locationCounter;
            relocation.isInstruction = true;

            relocationTable.addRelocation(currentSection, relocation);

            return literalAddress;
        } 
        else
        {
            unsigned int placeholderAddress = 0;

            RelocationEntry relocation;
            relocation.offset = locationCounter;
            relocation.type = RelocationType::ABSOLUTE;
            relocation.symbolName = operand;
            relocation.addend = 0;
            relocation.isInstruction = true;

            relocationTable.addRelocation(currentSection, relocation);

            string placeholderStr = to_string(placeholderAddress);
            unsigned int literalAddress = literalPool.addLiteral(currentSection, placeholderStr, locationCounter);

            return literalAddress;
        }
    }
}

void assemblyParser::resolveForwardReferences(string symbolName)
{
    unsigned int symbolAddress = symbolTable.getSymbolAddress(symbolName);
    if (symbolAddress == -1)
    {
        return;
    }

    auto& relocations = relocationTable.getRelocations(currentSection);
    auto& sectionData = sectionTable.getSection(currentSection).data;

    for (const auto& entry : relocations)
    {
        if (entry.symbolName == symbolName)
        {
            int finalAddress = 0;

            finalAddress = symbolAddress - entry.offset;

            vector<uint8_t> instruction(4);
            instruction[0] = sectionData[entry.offset];
            instruction[1] = sectionData[entry.offset + 1];
            instruction[2] = sectionData[entry.offset + 2];
            instruction[3] = sectionData[entry.offset + 3];

            uint32_t instructionValue = 
                static_cast<uint32_t>(instruction[0] << 24) |
                (static_cast<uint32_t>(instruction[1]) << 16) |
                (static_cast<uint32_t>(instruction[2]) << 8) |
                (static_cast<uint32_t>(instruction[3]));

            instructionValue &= 0xFFFFF000;

            instructionValue |= (finalAddress & 0xFFF);

            instruction[0] = (instructionValue >> 24) & 0xFF;
            instruction[1] = (instructionValue >> 16) & 0xFF;
            instruction[2] = (instructionValue >> 8) & 0xFF;
            instruction[3] = (instructionValue) & 0xFF;

            sectionData[entry.offset] = instruction[0];
            sectionData[entry.offset + 1] = instruction[1];
            sectionData[entry.offset + 2] = instruction[2];
            sectionData[entry.offset + 3] = instruction[3];

            relocationTable.eraseRelocation(currentSection, symbolName, entry.offset);
        }
    }
}

string toHex(unsigned int value)
{
    stringstream stream;
    stream << hex << setw(2) << setfill('0') << value;
    return stream.str();
}

void assemblyParser::writeOutputTxt()
{
    outputFile << "------ Symbol Table ------\n\n";
    outputFile << "Name           Section           Type           Value\n";
    outputFile << "-------------------------------------------------------\n";

    for (const auto& entry : symbolTable.getAllSymbols())
    {
        if(entry.section == entry.name)
        {
            const auto& data = sectionTable.getSection(entry.name).data;

            uint32_t literalPoolSize = literalPool.getLiteralPoolSize(entry.name);
            uint32_t sectionSize = data.size() + literalPoolSize;

            std::stringstream ss;
            ss << "0x" << std::hex << std::setw(2) << std::setfill('0') << sectionSize;

            outputFile << std::setw(16) << std::left << entry.name
                    << std::setw(16) << std::left << entry.name
                    << std::setw(15) << std::left << "LOCAL"
                    << ss.str() << "\n";
        }
        else if(entry.type == SymbolType::EXTERNAL || entry.global || entry.type == SymbolType::LOCAL)
        {
            outputFile << std::setw(16) << std::left << entry.name
                    << std::setw(16) << std::left << (entry.section.empty() ? "NONE" : entry.section)
                    << std::setw(15) << std::left << (entry.type == SymbolType::EXTERNAL ? "EXTERN" : (entry.global ? "GLOBAL" : "LOCAL"))
                    << (entry.type == SymbolType::EXTERNAL || !entry.defined ? "?" : "0x" + toHex(entry.address)) << "\n";
        }
    }

    outputFile << "\n\n------ Relocations ------\n\n";

    for (const auto& section : sectionTable.getSectionNames())
    {
        outputFile << section << ":\n";
        outputFile << "Offset      Symbol         Instruction\n";
        outputFile << "----------------------------------------\n";

        for (const auto& relocation : relocationTable.getRelocations(section))
        {
            outputFile << std::setw(10) << std::left << std::hex << relocation.offset
                       << std::setw(16) << std::left << relocation.symbolName
                       << std::setw(16) << std::left << (relocation.isInstruction ? "true" : "false") << "\n";
        }
        outputFile << "\n";
    }

    outputFile << "\n\n------ Code ------\n\n";

    for (const auto& section : sectionTable.getSectionNames())
    {
        outputFile << section << ":\n";

        literalPool.resolveLiteralsInSection(section, sectionTable.getSection(section).data);

        const auto& data = sectionTable.getSection(section).data;

        for (size_t i = 0; i < data.size(); i += 16)
        {
            for (size_t j = 0; j < 16 && i + j < data.size(); ++j)
            {
                outputFile << std::setw(2) << std::setfill('0') << std::right << std::hex
                           << (static_cast<int>(data[i + j]) & 0xFF) << " ";
            }
            outputFile << "\n";
        }
        outputFile << "\n";
    }
}

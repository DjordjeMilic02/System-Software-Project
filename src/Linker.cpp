#include "../inc/Linker.hpp"
#include <iostream>
#include <fstream>
#include <algorithm>

Linker::Linker(const CommandLineParser& parser): parser(parser), outputFile(parser.getOutputFileName()), hexMode(parser.isHexMode())
{
    sectionAddresses = parser.getSectionsToPlace();
}

void Linker::loadObjectFiles()
{
    for (const auto& filename : parser.getInputFiles())
    {
        InputFile inputFile;
        inputFile.filename = filename;
        parseInputFile(filename, inputFile);
        inputFiles.push_back(inputFile);
    }
}

void Linker::parseInputFile(const std::string& filename, InputFile& inputFile)
{
    std::ifstream file(filename, std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("Could not open file: " + filename);
    }

    std::string line;
    enum class ParseState
    {
        None,
        SymbolTable,
        Relocations,
        Code
    } state = ParseState::None;

    std::string currentSection;

    while (std::getline(file, line))
    {
        if (line.find("------ Symbol Table ------") != std::string::npos)
        {
            state = ParseState::SymbolTable;
            std::getline(file, line);
            std::getline(file, line);
        } else if (line.find("------ Relocations ------") != std::string::npos)
        {
            state = ParseState::Relocations;
            std::getline(file, line);
        } else if (line.find("------ Code ------") != std::string::npos)
        {
            state = ParseState::Code;
            std::getline(file, line);
        } else
        {
            switch (state)
            {
                case ParseState::SymbolTable:
                {
                    std::string name, section, type, valueStr;
                    int32_t value;
                    bool defined;
                    std::istringstream iss(line);
                    if (!(iss >> name >> section >> type >> valueStr)) {
                        continue;
                    }
                    if (valueStr == "?") {
                        value = -1;
                        defined = false;
                    } else {
                        value = std::stoul(valueStr, nullptr, 16);
                        defined = true;
                    }
                    SymbolType symbolType = (type == "EXTERN") ? SymbolType::EXTERNAL :
                                            (type == "GLOBAL") ? SymbolType::GLOBAL :
                                            (type == "LOCAL") ? SymbolType::LOCAL: SymbolType::LABEL;

                    bool isGlobal = (type == "GLOBAL");
                    inputFile.symbolTable.addSymbol(name, section, value, symbolType, isGlobal, value, defined);
                    if(name == section)
                    {   
                      inputFile.sectionTable.addSection(name);
                      globalSectionTable.addSection(GlobalSectionTable::Section(name, filename));
                    }
                    break;
                }
                case ParseState::Relocations:
                {
                    if (line.back() == ':') {
                        currentSection = line.substr(0, line.size() - 1);
                    } else
                    {
                        std::istringstream iss(line);
                        uint32_t offset;
                        std::string symbol;
                        std::string isInstructionStr;

                        if (!(iss >> std::hex >> offset >> symbol >> isInstructionStr)) {
                            continue;
                        }

                        RelocationEntry relocation;
                        relocation.offset = offset;
                        relocation.type = RelocationType::ABSOLUTE;
                        relocation.symbolName = symbol;
                        relocation.addend = 0;
                        relocation.isInstruction = (isInstructionStr == "true")?true:false;

                        inputFile.relocationTable.addRelocation(currentSection, relocation);
                    }
                    break;
                }
                case ParseState::Code:
                {
                    if (line.back() == ':') {
                        currentSection = line.substr(0, line.size() - 1);
                    } else {
                        std::istringstream iss(line);
                        std::vector<uint8_t> bytes;

                        std::string byteStr;
                        while (iss >> byteStr) {
                            uint8_t byte = std::stoul(byteStr, nullptr, 16);
                            bytes.push_back(byte);
                        }

                        inputFile.sectionTable.writeDataToSection(currentSection, bytes);
                        globalSectionTable.writeDataToSection(currentSection, filename, bytes);
                        
                    }
                    break;
                }
                case ParseState::None:
                    break;
            }
        }
    }

    file.close();
}

void Linker::testParseInputFile(const std::string& filename)
{
    InputFile inputFile;
    parseInputFile(filename, inputFile);
    std::cout << "Parsed Symbol Table:" << std::endl;
    for (const auto& symbol : inputFile.symbolTable.getAllSymbols()) {
        std::cout << "Name: " << symbol.name 
                  << ", Section: " << symbol.section 
                  << ", Type: " << (symbol.global ? "GLOBAL" : (symbol.type == SymbolType::EXTERNAL ? "EXTERN" : "LOCAL"))
                  << ", Value: 0x" << std::hex << symbol.value << std::dec << std::endl;
    }

    std::cout << "\nParsed Relocation Table:" << std::endl;
    for (const auto& sectionName : inputFile.sectionTable.getSectionNames()) {
        std::cout << sectionName << ":" << std::endl;
        for (const auto& relocation : inputFile.relocationTable.getRelocations(sectionName)) {
            std::cout << "Offset: 0x" << std::hex << relocation.offset
                      << ", Symbol: " << relocation.symbolName
                      << ", Instruction: " << (relocation.isInstruction ? "true" : "false") << std::dec << std::endl;
        }
    }

    std::cout << "\nParsed Sections:" << std::endl;
    for (const auto& sectionName : inputFile.sectionTable.getSectionNames()) {
        std::cout << sectionName << ":" << std::endl;
        const auto& data = inputFile.sectionTable.getSection(sectionName).data;
        for (size_t i = 0; i < data.size(); ++i) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]) << " ";
            if ((i + 1) % 16 == 0) std::cout << std::endl;
        }
        std::cout << std::dec << std::endl;
    }
}

void Linker::placeSections()
{
    unsigned int nextAvailableAddress = 0;
    for (const auto& sectionPair : globalSectionTable.getAllSections())
    {
        for (auto& section : globalSectionTable.getSections(sectionPair.name))
        {
            unordered_map<string, uint32_t> sections = parser.getSectionsToPlace();
            if (sections.find(section.name) != sections.end())
            {
                section.assignedAddress = sections[section.name];
                section.hasPlaceOption = true;

                //cout << section.name << " PLACED AT FIXED ADDRESS: " << section.assignedAddress << endl;
            }
        }
    }

    for (const auto& sectionPair : globalSectionTable.getAllSections())
    {
        for (auto& section : globalSectionTable.getSections(sectionPair.name))
        {
            if (!section.hasPlaceOption && section.assignedAddress == -1)
            {
                unsigned int proposedAddress = nextAvailableAddress;
                bool overlaps = false;

                do {
                    overlaps = false;

                    for (const auto& placedSectionPair : globalSectionTable.getAllSections())
                    {
                        for (const auto& placedSection : globalSectionTable.getSections(placedSectionPair.name))
                        {
                            if (placedSection.assignedAddress != -1)
                            {
                                unsigned int placedSectionEnd = placedSection.assignedAddress + placedSection.data.size();
                                unsigned int sectionEnd = proposedAddress + section.data.size();

                                if (!(sectionEnd <= placedSection.assignedAddress || proposedAddress >= placedSectionEnd))
                                {
                                    overlaps = true;
                                    proposedAddress = placedSectionEnd;
                                    break;
                                }
                            }
                        }
                        if (overlaps) break;
                    }

                } while (overlaps);

                section.assignedAddress = proposedAddress;
                //cout << section.name << " PLACED SECTION OF SIZE: " << section.data.size() << " AT ADDRESS: " << proposedAddress << endl;

                nextAvailableAddress = proposedAddress + section.data.size();
            }
        }
    }
}


void Linker::defineAllSymbols()
{
    for (auto& inputFile : inputFiles)
    {
        //std::cout << "Processing file: " << inputFile.filename << std::endl;

        for (auto& symbolPair : inputFile.symbolTable.symbols)
        {
            Symbol& symbol = symbolPair.second;

            //std::cout << "Checking symbol: " << symbol.name << " (Defined: " << symbol.defined << ", Global: " << symbol.global << ", Section: " << symbol.section << ", Address: 0x" << std::hex << symbol.address << ")" << std::endl;

            if (symbol.name == symbol.section)
            {
                auto globalSectionIt = globalSectionTable.sectionsMap.find(symbol.section);

                if (globalSectionIt != globalSectionTable.sectionsMap.end())
                {
                    for (const auto& section : globalSectionIt->second)
                    {
                        if (section.file == inputFile.filename)
                        {
                            symbol.address = section.assignedAddress;
                            symbol.value = section.data.size();
                            symbol.defined = true;
                            symbol.done = true;

                            //std::cout << "Placed section symbol: " << symbol.name << " at address: 0x" << std::hex << symbol.address << " with size: " << symbol.value << std::endl;
                            break;
                        }
                    }
                } else {
                    throw std::runtime_error("Section not found for symbol: " + symbol.section);
                }
            }
            else if (symbol.defined)
            {
                auto globalSectionIt = globalSectionTable.sectionsMap.find(symbol.section);

                if (globalSectionIt != globalSectionTable.sectionsMap.end())
                {
                    for (const auto& section : globalSectionIt->second)
                    {
                        if (section.file == inputFile.filename)
                        {
                            unsigned int finalAddress = symbol.address + section.assignedAddress;
                            symbol.address = finalAddress;
                            symbol.done = true;

                            //std::cout << "final addresss: " << symbol.name << " is 0x" << std::hex << finalAddress << std::endl;
                            break;
                        }
                    }
                } else {
                    throw std::runtime_error("Section not found for symbol: " + symbol.section);
                }
            }
        }
    }

    for (auto& inputFile : inputFiles)
    {
        //std::cout << "Processing input file: " << inputFile.filename << std::endl;
        for (auto& symbolPair : inputFile.symbolTable.symbols)
        {
            Symbol& symbol = symbolPair.second;

            if (!symbol.defined && !symbol.done)
            {
                Symbol* definitionSymbol = nullptr;

                for (auto& otherFile : inputFiles)
                {
                    if (&otherFile == &inputFile) continue;

                    auto it = otherFile.symbolTable.symbols.find(symbol.name);
                    if (it != otherFile.symbolTable.symbols.end())
                    {
                        Symbol& otherSymbol = it->second;

                        if (otherSymbol.defined)
                        {
                            if (definitionSymbol)
                            {
                                throw std::runtime_error("Multiple definitions found for symbol: " + symbol.name);
                            }
                            definitionSymbol = &otherSymbol;
                            //std::cout << "Found definition for symbol: " << symbol.name << " in file: " << otherFile.filename << " at address: 0x" << std::hex << definitionSymbol->address << std::endl;
                        }
                    }
                }

                if (definitionSymbol)
                {
                    symbol.address = definitionSymbol->address;
                    symbol.defined = true;
                    symbol.done = true;

                    //std::cout << "Resolved external symbol: " << symbol.name << " to address: 0x" << std::hex << symbol.address << std::endl;
                } else {
                    std::cerr << "Error: Undefined external symbol: " << symbol.name << std::endl;
                    throw std::runtime_error("Undefined external symbol: " + symbol.name);
                }
            }
        }
    }
}

void Linker::resolveForwardReferences(std::vector<InputFile>& inputFiles, GlobalSectionTable& globalSectionTable)
{

    for (auto& inputFile : inputFiles)
    {
        //std::cout << "Processing file: " << inputFile.filename << std::endl;

        auto& symbolTable = inputFile.symbolTable;
        auto& relocationTable = inputFile.relocationTable;
        auto& sectionTable = inputFile.sectionTable;

        auto allRelocations = relocationTable.getAllEntries();

        for (const auto& sectionPair : allRelocations)
        {
            const std::string& sectionName = sectionPair.first;
            const std::vector<RelocationEntry>& relocations = sectionPair.second;

            //std::cout << "Processing section: " << sectionName << std::endl;

            auto sectionIt = sectionTable.sections.find(sectionName);
            if (sectionIt == sectionTable.sections.end())
            {
                std::cerr << "Error: Section not found: " << sectionName << std::endl;
                throw std::runtime_error("Section not found: " + sectionName);
            }
            auto& sectionData = sectionIt->second.data;

            for (const auto& relocationEntry : relocations)
            {
                const std::string& symbolName = relocationEntry.symbolName;
                unsigned int instructionOffset = relocationEntry.offset;

                //std::cout << endl << "Relocating symbol: " << symbolName << " at offset: 0x" << std::hex << instructionOffset << std::endl;

                if (!symbolTable.isSymbolDefined(symbolName))
                {
                    std::cerr << "Error: Undefined symbol: " << symbolName << std::endl;
                    throw std::runtime_error("Undefined symbol: " + symbolName);
                }

                unsigned int symbolAddress = symbolTable.getSymbolAddress(symbolName);
                //std::cout << "Symbol address for " << symbolName << " is 0x" << std::hex << symbolAddress << std::endl;

                int i = (sectionData[instructionOffset + 2] & 0x0F) | (sectionData[instructionOffset + 3] & 0xFF);
                //cout << (sectionData[instructionOffset + 2] & 0x0F)  << (sectionData[instructionOffset + 3] & 0xFF) << "!!!" << i << endl;
                uint32_t offsetInSection = instructionOffset + i;
                //std::cout << "Calculated offset in section: 0x" << std::hex << offsetInSection << std::endl;

                if (offsetInSection + 3 >= sectionData.size())
                {
                    std::cerr << "Error: Offset 0x" << std::hex << offsetInSection << " is out of bounds for section data size: " << sectionData.size() << std::endl;
                    throw std::runtime_error("Offset out of bounds for section data.");
                }

                std::vector<uint8_t> address(4);
                address[0] = static_cast<uint8_t>(symbolAddress & 0xFF);
                address[1] = static_cast<uint8_t>((symbolAddress >> 8) & 0xFF);
                address[2] = static_cast<uint8_t>((symbolAddress >> 16) & 0xFF);
                address[3] = static_cast<uint8_t>((symbolAddress >> 24) & 0xFF);

                globalSectionTable.writeDataToSectionA(sectionIt->second.name, inputFile.filename, address, offsetInSection);

                sectionData[offsetInSection] = static_cast<uint8_t>(symbolAddress & 0xFF);
                sectionData[offsetInSection + 1] = static_cast<uint8_t>((symbolAddress >> 8) & 0xFF);
                sectionData[offsetInSection + 2] = static_cast<uint8_t>((symbolAddress >> 16) & 0xFF);
                sectionData[offsetInSection + 3] = static_cast<uint8_t>((symbolAddress >> 24) & 0xFF);

                //std::cout << "Updated section data at offset 0x" << std::hex << offsetInSection << " with symbol address 0x" << std::hex << symbolAddress << std::endl;
            }
        }
    }

}

void Linker::displaySectionPlacements() const
{
    for (const auto& section : globalSectionTable.getAllSections())
    {
        //std::cout << "Section '" << section.name << "' is placed at address 0x" << std::hex << section.assignedAddress << std::dec << " With size: " << section.data.size() << std::endl;
    }
}

void Linker::generateOutputFile(const std::string& outputFilename, GlobalSectionTable& globalSectionTable)
{
    auto sections = globalSectionTable.getAllSections();

    std::sort(sections.begin(), sections.end(), [](const GlobalSectionTable::Section& a, const GlobalSectionTable::Section& b) {
        return a.assignedAddress < b.assignedAddress;
    });

    std::ofstream outputFile(outputFilename);
    if (!outputFile) {
        throw std::runtime_error("Failed to open output file: " + outputFilename);
    }

    for (const auto& section : sections)
    {
        if (section.assignedAddress >= 0)
        {
            uint32_t currentAddress = section.assignedAddress;

            const size_t bytesPerLine = 16;
            for (size_t i = 0; i < section.data.size(); i += bytesPerLine)
            {
                outputFile << std::hex << std::setw(8) << std::setfill('0') << currentAddress << ": ";

                for (size_t j = 0; j < bytesPerLine && i + j < section.data.size(); ++j)
                {
                    outputFile << std::hex << std::setw(2) << std::setfill('0')
                               << static_cast<int>(section.data[i + j]) << " ";
                }
                outputFile << std::endl;
                currentAddress += bytesPerLine;
            }
        }
    }
    outputFile.close();
}
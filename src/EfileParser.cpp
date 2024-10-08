#include "../inc/EfileParser.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>


fileParser::fileParser() {}

void fileParser::loadMemoryFromFile(const string& filename)
{
    ifstream file(filename);
    if (!file.is_open())
    {
        throw runtime_error("Failed to open file: " + filename);
    }

    string line;
    while (getline(file, line))
    {
        istringstream iss(line);
        string addressStr, byteStr;

        iss >> addressStr;
        addressStr.pop_back();

        uint32_t address = stoul(addressStr, nullptr, 16);

        while (iss >> byteStr)
        {
            uint8_t byte = static_cast<uint8_t>(stoul(byteStr, nullptr, 16));
            memory[address++] = byte;
        }
    }

    file.close();
}
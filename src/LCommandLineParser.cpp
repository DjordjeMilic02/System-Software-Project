#include "../inc/LCommandLineParser.hpp"
#include <iostream>
#include <sstream>
#include <stdexcept>

void CommandLineParser::parseArguments(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-o")
        {
            if (i + 1 < argc)
            {
                outputFileName = argv[++i];
            }
            else
            {
                throw std::invalid_argument("Output file name expected after -o");
            }
        }
        else if (arg.find("-place=") == 0)
        {
            size_t pos = arg.find('@');
            if (pos == std::string::npos)
            {
                throw std::invalid_argument("Invalid -place option format");
            }

            std::string section = arg.substr(7, pos - 7);
            std::string addressStr = arg.substr(pos + 1);
            uint32_t address = parseHexAddress(addressStr);
            sectionsToPlace[section] = address;
        }
        else if (arg == "-hex")
        {
            hexMode = true;
        }
        else
        {
            inputFiles.push_back(arg);
        }
    }

    if (!hexMode)
    {
        throw std::invalid_argument("-hex option is required");
    }

    if (inputFiles.empty())
    {
        throw std::invalid_argument("No input files provided");
    }
}

const std::string& CommandLineParser::getOutputFileName() const
{
    return outputFileName;
}

const std::unordered_map<std::string, uint32_t>& CommandLineParser::getSectionsToPlace() const
{
    return sectionsToPlace;
}

bool CommandLineParser::isHexMode() const
{
    return hexMode;
}

const std::vector<std::string>& CommandLineParser::getInputFiles() const
{
    return inputFiles;
    //\\s*,\\s*(shl|shr)\\s*\\$(" + number + ")$
}

uint32_t CommandLineParser::parseHexAddress(const std::string& str)
{
    uint32_t address = 0;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> address;
    if (ss.fail())
    {
        throw std::invalid_argument("Invalid hex address: " + str);
    }
    return address;
}
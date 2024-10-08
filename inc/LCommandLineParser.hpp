#ifndef COMMAND_LINE_PARSER_HPP
#define COMMAND_LINE_PARSER_HPP

#include <string>
#include <vector>
#include <unordered_map>

class CommandLineParser
{
public:
    void parseArguments(int argc, char* argv[]);

    const std::string& getOutputFileName() const;
    const std::unordered_map<std::string, uint32_t>& getSectionsToPlace() const;
    bool isHexMode() const;
    const std::vector<std::string>& getInputFiles() const;

private:
    std::string outputFileName = "out.txt";
    std::unordered_map<std::string, uint32_t> sectionsToPlace;
    bool hexMode = false;
    std::vector<std::string> inputFiles;

    uint32_t parseHexAddress(const std::string& str);
};

#endif // COMMAND_LINE_PARSER_HPP

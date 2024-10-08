#ifndef LITERAL_POOL_HPP
#define LITERAL_POOL_HPP

#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <iomanip>

using namespace std;

struct LiteralEntry {
    string value;
    uint32_t location;
    vector<uint32_t> references;
};

struct SectionLiteralPool {
    vector<LiteralEntry> literals;
    uint32_t sectionSize = 0;
};

class LiteralPool {
private:
    map<string, SectionLiteralPool> sections;

public:
    uint32_t addLiteral(const string& sectionName, const string& literal, uint32_t instructionLocation);
    void resolveLiteralsInSection(const string& sectionName, vector<uint8_t>& sectionData);
    void updateSectionSize(const string& sectionName, uint32_t newSize);
    void clearLiteralPoolForSection(const string& sectionName);
    uint32_t getLiteralPoolSize(const std::string& sectionName);
};

#endif // LITERAL_POOL_HPP
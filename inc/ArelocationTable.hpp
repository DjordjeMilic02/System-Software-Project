#ifndef RELOCATION_TABLE_HPP
#define RELOCATION_TABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>

using namespace std;

enum class RelocationType
{
    ABSOLUTE,
    PC_RELATIVE
};

struct RelocationEntry
{
    unsigned int offset;
    RelocationType type;
    string symbolName;
    unsigned int addend;
    bool isInstruction;
};

class RelocationTable
{
    public:
    void addRelocation(const string& sectionName, const RelocationEntry& entry);
    void eraseRelocation(const string& sectionName, const string& symbolName, int offset);

    const vector<RelocationEntry>& getRelocations(const string& sectionName) const;

    private:
    unordered_map<string, vector<RelocationEntry>> relocations;
};

#endif // RELOCATION_TABLE_HPP
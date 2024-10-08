#include "../inc/LrelocationTable.hpp"
#include "algorithm"

using namespace std;

void RelocationTable::addRelocation(const string& sectionName, const RelocationEntry& entry)
{
    relocations[sectionName].push_back(entry);
}

std::unordered_map<std::string, std::vector<RelocationEntry>> RelocationTable::getAllEntries() const
{
    return relocations;
}

void RelocationTable::eraseRelocation(const string& sectionName, const string& symbolName, int offset)
{
    auto& relocationEntries = relocations[sectionName];

    relocationEntries.erase(
        remove_if(relocationEntries.begin(), relocationEntries.end(),
                  [&](const RelocationEntry& entry) {
                      return entry.symbolName == symbolName && entry.offset == offset;
                  }),
        relocationEntries.end());
}

const vector<RelocationEntry>& RelocationTable::getRelocations(const string& sectionName) const
{
    auto it = relocations.find(sectionName);
    if (it != relocations.end())
    {
        return it->second;
    }
    else
    {
        static const vector<RelocationEntry> empty;
        return empty;
    }
}
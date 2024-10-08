#ifndef SECTION_TABLE_HPP
#define SECTION_TABLE_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>

using namespace std;

struct Section
{
    string name;
    unsigned int size;
    vector<uint8_t> data;
};

class SectionTable
{
  public:
    unordered_map<string, Section> sections;

    void addSection(const string& name);
    Section& getSection(const string& name);
    bool sectionExists(const string& name) const;
    void writeDataToSection(const string& name, const vector<uint8_t>& data);
    void writeDataToSection(const string& name, unsigned int offset, const vector<uint8_t>& data);
    unsigned int getSectionSize(const string& name) const;
    vector<string> getSectionNames() const;
};

#endif // SECTION_TABLE_HPP
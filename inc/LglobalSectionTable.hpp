#ifndef GLOBAL_SECTION_TABLE_HPP
#define GLOBAL_SECTION_TABLE_HPP

#include <unordered_map>
#include <vector>
#include <string>
#include <cstdint>

using namespace std;

class GlobalSectionTable {
public:
    // Defines the Section struct
    struct Section {
        std::string name;
        std::vector<uint8_t> data;
        bool hasPlaceOption = false;
        unsigned int specifiedAddress = 0;
        unsigned int assignedAddress = -1;
        string file;

        Section(const std::string& name) : name(name) {}
        Section(const string& name, const string& file) : name(name), file(file) {}
    };

    void writeDataToSection(const std::string& name, const string& file, const std::vector<uint8_t>& bytes);

    void writeDataToSectionA(const std::string& name, const string& file, const std::vector<uint8_t>& bytes, int address);

    void addSection(const Section& section);

    std::vector<Section>& getSections(const std::string& sectionName);

    bool hasSection(const std::string& sectionName) const;

    std::vector<Section> getAllSections() const;

    std::unordered_map<std::string, std::vector<Section>> sectionsMap;
};

#endif // GLOBAL_SECTION_TABLE_HPP

#include "../inc/LglobalSectionTable.hpp"
#include <iostream>
using namespace std;

void GlobalSectionTable::addSection(const Section& section)
{
    sectionsMap[section.name].push_back(section);
}

std::vector<GlobalSectionTable::Section>& GlobalSectionTable::getSections(const std::string& sectionName)
{
    return sectionsMap[sectionName];
}

bool GlobalSectionTable::hasSection(const std::string& sectionName) const
{
    return sectionsMap.find(sectionName) != sectionsMap.end();
}

std::vector<GlobalSectionTable::Section> GlobalSectionTable::getAllSections() const
{
    std::vector<Section> allSections;
    for (const auto& pair : sectionsMap)
    {
        for (const auto& section : pair.second)
        {
            allSections.push_back(section);
        }
    }
    return allSections;
}

void GlobalSectionTable::writeDataToSection(const std::string& name, const std::string& file, const std::vector<uint8_t>& bytes)
{
    auto sectionIt = sectionsMap.find(name);
    if (sectionIt != sectionsMap.end())
    {

        bool sectionFound = false;
        for (int i = 0; i < sectionIt->second.size(); i++)
        {
            if (sectionIt->second[i].file == file)
            {
                sectionIt->second[i].data.insert(sectionIt->second[i].data.end(), bytes.begin(), bytes.end());
                //cout << "Write successful to section: " << name << " for file: " << file << endl;
                sectionFound = true;
                break;
            }
        }

        if (!sectionFound)
        {
            //cout << "No section with file name '" << file << "' found in section group: " << name << endl;
        }
    } else
    {
        cerr << "Error: Section group not found: " << name << endl;
        throw std::runtime_error("Section not found: " + name);
    }
}

void GlobalSectionTable::writeDataToSectionA(const std::string& name, const std::string& file, const std::vector<uint8_t>& bytes, int address)
{
    //cout << "Attempting to write data to section: " << name << " for file: " << file << endl;

    auto sectionIt = sectionsMap.find(name);
    if (sectionIt != sectionsMap.end())
    {
        //cout << "Section group found: " << name << endl;

        bool sectionFound = false;
        for (int i = 0; i < sectionIt->second.size(); i++)
        {
            //cout << "Checking section " << i << " with file name: " << sectionIt->second[i].file << endl;
            if (sectionIt->second[i].file == file)
            {
                //cout << "Section with file name '" << file << "' found." << endl;

                if (sectionIt->second[i].data.size() < address + bytes.size())
                {
                    //cout << "Resizing section data from " << sectionIt->second[i].data.size() << " to " << address + bytes.size() << endl;
                    sectionIt->second[i].data.resize(address + bytes.size(), 0);
                }

                //cout << "Writing " << bytes.size() << " bytes to address " << address << " in section." << endl;
                std::copy(bytes.begin(), bytes.end(), sectionIt->second[i].data.begin() + address);

                //cout << "Data written: ";
                for (int j = 0; j < bytes.size(); ++j)
                {
                    //cout << std::hex << static_cast<int>(bytes[j]) << " ";
                }
                //cout << endl;

                sectionFound = true;
                break;
            }
        }

        if (!sectionFound)
        {
            //cout << "No section with file name '" << file << "' found in section group: " << name << endl;
        }
    } else {
        cerr << "Error: Section group not found: " << name << endl;
        throw std::runtime_error("Section not found: " + name);
    }
}
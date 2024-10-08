#include "../inc/LsectionTable.hpp"
#include <stdexcept>

void SectionTable::addSection(const string& name)
{
  if (sections.find(name) == sections.end())
  {
    sections[name] = Section{name, 0, {}};
  }
  else
  {
    throw runtime_error("Section already exists: " + name);
  }
}

Section& SectionTable::getSection(const string& name)
{
  if (sections.find(name) != sections.end())
  {
    return sections[name];
  }
  else
  {
    throw runtime_error("Section not found: " + name);
  }
}

bool SectionTable::sectionExists(const string& name) const
{
  return sections.find(name) != sections.end();
}

void SectionTable::writeDataToSection(const string& name, const vector<uint8_t>& data)
{
  if (!sectionExists(name)) {
      throw runtime_error("Section does not exist: " + name);
  }
  sections[name].data.insert(sections[name].data.end(), data.begin(), data.end());
  sections[name].size += data.size();
}

void SectionTable::writeDataToSection(const string& name, unsigned int offset, const vector<uint8_t>& data)
{
  if (!sectionExists(name))
  {
      throw runtime_error("Section does not exist: " + name);
  }
  
  Section& section = sections[name];
  if (offset + data.size() > section.data.size())
  {
      throw runtime_error("Attempt to write beyond section size");
  }
  
  copy(data.begin(), data.end(), section.data.begin() + offset);
}

unsigned int SectionTable::getSectionSize(const string& name) const
{
  auto it = sections.find(name);
  if (it != sections.end())
  {
      return it->second.size;
  }
  throw runtime_error("Section not found: " + name);
}

vector<string> SectionTable::getSectionNames() const
{
  vector<string> sectionNames;
  for (const auto& pair : sections)
  {
      sectionNames.push_back(pair.first);
  }
  return sectionNames;
}
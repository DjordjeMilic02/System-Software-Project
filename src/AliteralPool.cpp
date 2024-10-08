#include "../inc/AliteralPool.hpp"
#include <iostream>

uint32_t LiteralPool::addLiteral(const string& sectionName, const string& literal, uint32_t instructionLocation)
{
    auto& sectionPool = sections[sectionName];

    uint32_t literalAddress = sectionPool.sectionSize;

    LiteralEntry newEntry = {literal, literalAddress, {instructionLocation}};
    sectionPool.literals.push_back(newEntry);

    sectionPool.sectionSize += 4;

    return literalAddress;
}

void LiteralPool::resolveLiteralsInSection(const string& sectionName, vector<uint8_t>& sectionData)
{
    auto& sectionPool = sections[sectionName];

    for (auto& entry : sectionPool.literals) {
        uint32_t literalValue = static_cast<uint32_t>(stoul(entry.value));

        entry.location = sectionData.size();

        sectionData.push_back(static_cast<uint8_t>((literalValue) & 0xFF));
        sectionData.push_back(static_cast<uint8_t>((literalValue >> 8) & 0xFF));
        sectionData.push_back(static_cast<uint8_t>((literalValue >> 16) & 0xFF));
        sectionData.push_back(static_cast<uint8_t>((literalValue >> 24) & 0xFF));

        for (auto& refLocation : entry.references) {
            uint32_t displacement = entry.location - refLocation;
            uint32_t displacementMasked = displacement & 0xFFF;
            sectionData[refLocation + 3] = displacementMasked & 0xFF;
            sectionData[refLocation + 2] = (sectionData[refLocation + 2] & 0xF0) | ((displacementMasked >> 8) & 0x0F);
        }
    }
}

void LiteralPool::updateSectionSize(const string& sectionName, uint32_t newSize)
{
    sections[sectionName].sectionSize = newSize;
}

void LiteralPool::clearLiteralPoolForSection(const string& sectionName)
{
    sections.erase(sectionName);
}

uint32_t LiteralPool::getLiteralPoolSize(const std::string& sectionName)
{
    auto it = sections.find(sectionName);
    if (it != sections.end()) {
        return it->second.sectionSize;
    }
    return 0;
}
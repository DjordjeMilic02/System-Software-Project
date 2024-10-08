#include "../inc/LsymbolTable.hpp"
#include <stdexcept>
#include <vector>

using namespace std;

bool SymbolTable::addSymbol(const string& name, string sectionName, unsigned int address, SymbolType type, bool global, int value, bool defined)
{
    if (symbols.find(name) != symbols.end())
    {
        return false;
    }

    Symbol symbol = {name, sectionName, address, type, defined, global, value, false};
    symbols[name] = symbol;

    return true;
}

void SymbolTable::defineSymbol(const string& name, string sectionName, unsigned int address)
{
    if (address > 0xFFF)
    {
        throw runtime_error("Address out of range: must fit within 12 bits.");
    }

    auto it = symbols.find(name);
    if (it != symbols.end())
    {
        it->second.address = address;
        it->second.defined = true;
        it->second.value = address;
    } 
    else
    {
        symbols[name] = {name, sectionName, address, SymbolType::LABEL, true, false};
        symbols[name].value = address;
    }
}
bool SymbolTable::isSymbolDefined(const string& name) const
{
    auto it = symbols.find(name);
    return it != symbols.end() && it->second.defined;
}

unsigned int SymbolTable::getSymbolAddress(const string& name) const
{
    auto it = symbols.find(name);
    if (it != symbols.end() && it->second.defined)
    {
        return it->second.address;
    }
    return -1;
}

void SymbolTable::setExternalSymbol(const string& name, string sectionName)
{
    if (symbols.find(name) != symbols.end())
    {
        symbols[name].type = SymbolType::EXTERNAL;
    } else
    {
        symbols[name] = {name, sectionName, 0, SymbolType::EXTERNAL, false, true};
    }
}

bool SymbolTable::isGlobalSymbol(const string& name) const
{
    auto it = symbols.find(name);
    return it != symbols.end() && it->second.global;
}

vector<Symbol> SymbolTable::getAllSymbols() const {
    vector<Symbol> symbolList;
    for (const auto& pair : symbols) {
        symbolList.push_back(pair.second);
    }
    return symbolList;
}
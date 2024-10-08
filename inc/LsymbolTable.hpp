#ifndef SYMBOL_TABLE_HPP
#define SYMBOL_TABLE_HPP

#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

enum class SymbolType
{
    LABEL,
    VARIABLE,
    EXTERNAL,
    SECTION,
    GLOBAL,
    LOCAL,
};

struct Symbol
{
    string name;
    string section;
    unsigned int address;
    SymbolType type;
    bool defined;
    bool global;
    int value;
    bool done;
};

class SymbolTable
{
public:
    unordered_map<string, Symbol> symbols;
    
    bool addSymbol(const string& name, string sectionName, unsigned int address, SymbolType type, bool global, int value, bool defined);
    void defineSymbol(const string& name, string sectionName, unsigned int address);
    bool isSymbolDefined(const string& name) const;
    unsigned int getSymbolAddress(const string& name) const;
    void setExternalSymbol(const string& name, string sectionName);
    bool isGlobalSymbol(const string& name) const;
    vector<Symbol> getAllSymbols() const;
};

#endif // SYMBOL_TABLE_HPP
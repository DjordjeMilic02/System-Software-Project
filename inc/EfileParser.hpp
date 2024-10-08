#ifndef fileParser_HPP
#define fileParser_HPP

#include <string>
#include <map>
#include <cstdint>

using namespace std;

class fileParser {
public:
    fileParser();
    void loadMemoryFromFile(const string& filename);
    map<uint32_t, uint8_t> memory;
};

#endif // fileParser_HPP
#include "../inc/EfileParser.hpp"
#include <iostream>
#include "../inc/EEmulator.hpp"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./emulator <file.hex>" << std::endl;
        return 1;
    }

    try {
        fileParser parser;
        parser.loadMemoryFromFile(argv[1]);
        Emulator emulator(parser);
        emulator.run();
    } catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
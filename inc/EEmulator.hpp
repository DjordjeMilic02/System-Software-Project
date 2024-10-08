#ifndef EMULATOR_HPP
#define EMULATOR_HPP

#include <cstdint>
#include <array>
#include "EfileParser.hpp"
#include <vector>

class Emulator
{
    public:
        Emulator(fileParser& parser);
        void run();
        int i = 0;
    private:
        std::array<uint32_t, 16> registers;
        std::array<uint32_t, 3> systemRegisters;
        std::map<uint32_t, uint8_t>& memory;
        bool running;

        uint32_t fetchMemoryValue32(uint32_t address);
        uint32_t fetchMemoryValue32I(uint32_t address);
        void storeMemoryValue32(uint32_t address, uint32_t value);
        void storeMemoryValue32I(uint32_t address, uint32_t value);
        void printState() const;
        void executeInstruction();
        void visualizeStack();
        void printStateToFile(const std::string& filename, const std::vector<uint8_t>& instruction) const;
};

#endif // EMULATOR_HPP
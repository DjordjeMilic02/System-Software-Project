#include "../inc/EEmulator.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <fstream>

using namespace std;

Emulator::Emulator(fileParser& parser) : memory(parser.memory), running(true)
{
    registers.fill(0);
    systemRegisters.fill(0);
    
    registers[15] = 0x40000000;
    registers[0] = 0x00;
}

void Emulator::run()
{
    //cout << "Start!" << endl;
    while (running)
    {
        executeInstruction();
    }
}

uint32_t Emulator::fetchMemoryValue32(uint32_t address)
{
  return (memory[address] << 24) |
               (memory[address + 1] << 16) |
               (memory[address + 2] << 8) |
               (memory[address + 3]);
}

uint32_t Emulator::fetchMemoryValue32I(uint32_t address)
{
  return (memory[address]) |
               (memory[address + 1] << 8) |
               (memory[address + 2] << 16) |
               (memory[address + 3] << 24);
}

void Emulator::storeMemoryValue32(uint32_t address, uint32_t value)
{
        memory[address] = (value >> 24) & 0xFF;
        memory[address + 1] = (value >> 16) & 0xFF;
        memory[address + 2] = (value >> 8) & 0xFF;
        memory[address + 3] = value & 0xFF;
}

void Emulator::storeMemoryValue32I(uint32_t address, uint32_t value)
{
        memory[address] = (value) & 0xFF;
        memory[address + 1] = (value >> 8) & 0xFF;
        memory[address + 2] = (value >> 16) & 0xFF;
        memory[address + 3] = (value >> 24) & 0xFF;
}

const uint32_t STACK_START_ADDRESS = 0xFFFFFEFE;

void Emulator::visualizeStack()
{
    uint32_t sp = registers[14];
    
    if (sp > STACK_START_ADDRESS)
    {
        std::cerr << "Error!" << std::endl;
    }

    std::cout << "Stack:" << std::endl;
    std::cout << "--------------------------------" << std::endl;

    for (uint32_t addr = STACK_START_ADDRESS; addr >= sp && addr >= 0xFFFFFED0; addr -= 4)
    {
        uint32_t value = fetchMemoryValue32(addr);

        std::cout << std::hex << std::setw(8) << std::setfill('0') << addr << ": " 
                  << std::hex << std::setw(8) << std::setfill('0') << value << std::endl;

        if (addr <= 4) break;
    }
    std::cout << "--------------------------------" << std::endl;
}

void Emulator::executeInstruction()
{
    uint32_t pc = registers[15];

    if (memory.find(pc) == memory.end() || memory.find(pc + 1) == memory.end() || memory.find(pc + 2) == memory.end() || memory.find(pc + 3) == memory.end())
    {
        throw runtime_error("Invalid memory access at address: " + to_string(pc));
    }

    vector<uint8_t> instruction =
    {
        memory[pc],
        memory[pc + 1],
        memory[pc + 2],
        memory[pc + 3]
    };

    printStateToFile("testing.txt", instruction);
    registers[15] += 4;
    switch (instruction[0] >> 4)
    {
        case 0x00:
        {
            running = false;
            cout << "Emulated processor executed halt instruction" << endl;
            printState();
            return;
            break;
        }
        case 0x01:
        {
            registers[14] -= 4;
            storeMemoryValue32I(registers[14], registers[15]);
            
            registers[14] -= 4;
            storeMemoryValue32I(registers[14], systemRegisters[0]);
            
            
            systemRegisters[2] = 4;
            systemRegisters[0] &= ~0x1;
            registers[15] = systemRegisters[1];
            break;
        }
        case 0x02:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint32_t D = ((instruction[2] & 0x0F) << 8) | instruction[3];

            registers[14] -= 4;
            storeMemoryValue32I(registers[14], registers[15]);
            
            

            uint32_t targetAddress = 0;

            switch (mode)
            {
                case 0x00:
                    targetAddress = registers[A] + registers[B] + fetchMemoryValue32I(registers[15]-4 + D);
                    break;
                case 0x01:
                    targetAddress = fetchMemoryValue32I(registers[A] + registers[B] + fetchMemoryValue32I(registers[15]-4 + D));
                    break;
                default:
                    throw runtime_error("Invalid call instruction");
            }
            registers[15] = targetAddress;

            break;
        }
        case 0x03:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;
            uint32_t D = (instruction[2] & 0x0F) << 8 | instruction[3];

            uint32_t targetAddress = registers[15];

            switch (mode)
            {
                case 0x00:
                    targetAddress = registers[A] + fetchMemoryValue32I(registers[15]-4 + D);
                    break;
                case 0x01:
                    if (registers[B] == registers[C])
                    {
                        targetAddress = registers[A] + fetchMemoryValue32I(registers[15]-4 + D);
                    } else {
                        break;
                    }
                    break;
                case 0x02:
                    if (registers[B] != registers[C])
                    {
                        targetAddress = registers[A] + fetchMemoryValue32I(registers[15]-4 + D);
                    } else {
                        break;
                    }
                    break;
                case 0x03:
                    if (static_cast<int32_t>(registers[B]) > static_cast<int32_t>(registers[C]))
                    {
                        targetAddress = registers[A] + fetchMemoryValue32I(registers[15]-4 + D);
                    } else {
                        break;
                    }
                    break;
                case 0x08:
                    targetAddress = fetchMemoryValue32I(registers[A] + fetchMemoryValue32I(registers[15]-4 + D));
                    break;
                case 0x09:
                    if (registers[B] == registers[C])
                    {
                        targetAddress = fetchMemoryValue32I(registers[A] + fetchMemoryValue32I(registers[15]-4 + D));
                    } else {
                        break;
                    }
                    break;
                case 0x0A:
                    if (registers[B] != registers[C])
                    {
                        targetAddress = fetchMemoryValue32I(registers[A] + fetchMemoryValue32I(registers[15]-4 + D));
                    } else {
                        break;
                    }
                    break;
                case 0x0B:
                    if (static_cast<int32_t>(registers[B]) > static_cast<int32_t>(registers[C]))
                    {
                        targetAddress = fetchMemoryValue32I(registers[A] + fetchMemoryValue32I(registers[15]-4 + D));
                    } else {
                        break;
                    }
                    break;
                default:
                    throw runtime_error("Invalid branch instruction");
            }

            registers[15] = targetAddress;

            break;
        }

        case 0x04:
        {
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;

            uint32_t temp = registers[B];
            registers[B] = registers[C];
            registers[C] = temp;
            registers[0] = 0;
            break;
        }
        case 0x05:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;

            switch (mode)
            {
                case 0x00:
                    registers[A] = registers[B] + registers[C];
                    break;

                case 0x01:
                    registers[A] = registers[B] - registers[C];
                    break;

                case 0x02:
                    registers[A] = registers[B] * registers[C];
                    break;

                case 0x03:
                    registers[A] = registers[B] / registers[C];
                    break;

                default:
                    throw runtime_error("Unknown arithmetic intruction");
            }
            registers[0] = 0;
            break;
        }
        case 0x06:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;

            switch (mode)
            {
                case 0x00:
                    registers[A] = ~registers[B];
                    break;

                case 0x01:
                    registers[A] = registers[B] & registers[C];
                    break;

                case 0x02:
                    registers[A] = registers[B] | registers[C];
                    break;

                case 0x03:
                    registers[A] = registers[B] ^ registers[C];
                    break;

                default:
                    throw runtime_error("Unknown logical instruction");
            }
            registers[0] = 0;
            break;
        }
        case 0x07:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;

            switch (mode)
            {
                case 0x00:
                    registers[A] = registers[B] << registers[C];
                    break;

                case 0x01:
                    registers[A] = registers[B] >> registers[C];
                    break;

                default:
                    throw runtime_error("Unknown shift instruction");
            }
            registers[0] = 0;
            break;
        }
        case 0x08:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;
            uint32_t D = (instruction[2] & 0x0F) << 8 | instruction[3];

            switch (mode)
            {
                case 0x00:
                {
                    uint32_t effectiveAddress = registers[A] + registers[B] + ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4));
                    storeMemoryValue32I(effectiveAddress, registers[C]);
                    break;
                }
                case 0x02:
                {
                    uint32_t intermediateAddress = registers[A] + registers[B] + ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4));
                    uint32_t effectiveAddress = fetchMemoryValue32I(intermediateAddress);
                    storeMemoryValue32I(effectiveAddress, registers[C]);
                    break;
                }
                case 0x01:
                {
                    registers[A] += fetchMemoryValue32I(D + registers[15]-4);
                    storeMemoryValue32I(registers[A], registers[C]);
                    break;
                }
                default:
                    throw std::runtime_error("Unknown store instruction");
            }
            registers[0] = 0;
            break;
        }
        case 0x09:
        {
            uint8_t mode = instruction[0] & 0x0F;
            uint8_t A = instruction[1] >> 4;
            uint8_t B = instruction[1] & 0x0F;
            uint8_t C = instruction[2] >> 4;
            uint32_t D = (instruction[2] & 0x0F) << 8 | instruction[3];

            switch (mode)
            {
                case 0x00:
                {
                    registers[A] = systemRegisters[B];
                    break;
                }
                case 0x01:
                {
                    registers[A] = registers[B] + ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4));
                    break;
                }
                case 0x02:
                {
                   registers[A] = fetchMemoryValue32I(registers[B] + registers[C] + ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4)));
                    break;
                }
                case 0x03:
                {
                    int pc1 = registers[15];
                    registers[A] = fetchMemoryValue32I(registers[B]);
                    registers[B] += fetchMemoryValue32I(pc1-4 + D);
                    break;
                }
                case 0x04:
                {
                    systemRegisters[A] = registers[B];
                    break;
                }
                case 0x05:
                {
                    systemRegisters[A] = systemRegisters[B] | ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4));
                    break;
                }
                case 0x06:
                {
                    uint32_t address = registers[B] + registers[C] + ((D == 0)?0:fetchMemoryValue32I(D + registers[15]-4));
                    systemRegisters[A] = fetchMemoryValue32I(address);
                    break;
                }
                case 0x07:
                {
                    int pc1 = registers[15];
                    uint32_t address = fetchMemoryValue32I(registers[B]);
                    systemRegisters[A] = address;
                    registers[B] += fetchMemoryValue32I(pc1-4 + D);
                    break;
                }
                default:
                    throw runtime_error("Unknown store instruction");
            }
            registers[0] = 0;
            break;
        }


        default:
            cerr << "Illegal instruction: 0x" << std::hex << std::setw(2) << std::setfill('0') << (int)instruction[0] << endl;
            running = false;
            break;
    }
    //visualizeStack();
}

void Emulator::printState() const
{
    std::cout << "Emulated processor state:" << std::endl;
    for (size_t i = 0; i < registers.size(); ++i)
    {
        std::cout << "r" << i << "=0x" << std::hex << std::setw(8) << std::setfill('0') << registers[i] << "    ";
        if ((i + 1) % 4 == 0) std::cout << std::endl;
    }
    std::cout << "CSR: " << systemRegisters[0] << " " << systemRegisters[1] << " " << systemRegisters[2] << std::endl;

    std::ofstream memoryDumpFile("memoryDump.txt", std::ios::app);
    if (!memoryDumpFile)
    {
        std::cerr << "Failed to open memoryDump.txt for writing!" << std::endl;
        return;
    }

    for (auto it = memory.begin(); it != memory.end(); )
    {
        uint32_t baseAddress = (it->first / 16) * 16;
        memoryDumpFile << std::hex << std::setw(8) << std::setfill('0') << baseAddress << ": ";

        for (uint32_t offset = 0; offset < 16; ++offset) {
            auto memIt = memory.find(baseAddress + offset);
            if (memIt != memory.end()) {
                memoryDumpFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(memIt->second) << " ";
            } else {
                memoryDumpFile << "   ";
            }
        }
        memoryDumpFile << std::endl;

        it = memory.lower_bound(baseAddress + 16);
    }
    memoryDumpFile.close();
}

void Emulator::printStateToFile(const std::string& filename, const std::vector<uint8_t>& instruction) const
{
    std::ofstream outFile(filename, std::ios::app);

    if (!outFile)
    {
        std::cerr << "Error: Could not open file " << filename << " for writing." << std::endl;
        return;
    }

    outFile << "Emulated processor state:" << std::endl;
    for (size_t i = 0; i < registers.size(); ++i)
    {
        outFile << "r" << i << "=0x" << std::hex << std::setw(8) << std::setfill('0') << registers[i] << "    ";
        if ((i + 1) % 4 == 0) outFile << std::endl;
    }

    outFile << std::endl << "System Registers: " << std::endl;
    outFile << systemRegisters[0] << " " << systemRegisters[1] << " " << systemRegisters[2] << std::endl;

    outFile << "Current Instruction: ";
    for (const auto& byte : instruction)
    {
        outFile << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " ";
    }
    outFile << std::endl;

    outFile << "-----------------------------" << std::endl;

    outFile.close();
}
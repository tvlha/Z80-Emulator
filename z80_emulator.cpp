#include <iostream>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip> // For hex formatting

#define NUM_CYCLES 1024

// Z80 CPU Structure
struct Z80_CPU
{
    uint16_t pc;              // Program Counter
    uint16_t sp;              // Stack Pointer
    uint8_t a, f;             // Accumulator and Flags
    uint8_t b, c;             // BC register pair
    uint8_t d, e;             // DE register pair
    uint8_t h, l;             // HL register pair
    uint8_t interrupt_enable; // Interrupt flag
    uint16_t ix, iy;          // Index registers
    uint8_t r;                // Refresh register
    uint8_t i;                // Interrupt register

    // Shadow registers
    uint8_t a_prime, f_prime;
    uint8_t b_prime, c_prime;
    uint8_t d_prime, e_prime;
};

static Z80_CPU cpu;
static uint8_t ram[65536];

// Function to set flags based on a result
void set_flags(uint8_t result)
{
    cpu.f = 0; // Reset flags
    if (result == 0)
        cpu.f |= 0x40; // Zero flag
    if (result & 0x80)
        cpu.f |= 0x80; // Sign flag
}

// Function to set flags for BIT operations
void set_bit_flags(uint8_t value, uint8_t bit)
{
    cpu.f &= ~(0x40 | 0x80 | 0x10); // Clear Zero, Sign, and Half carry flags
    if (!(value & (1 << bit)))
    {
        cpu.f |= 0x40; // Set Zero flag if bit is 0
    }
    else
    {
        cpu.f &= ~0x40; // Clear Zero flag if bit is 1
    }
    cpu.f |= 0x10; // Set Half carry flag
    if (bit == 7 && (value & 0x80))
    {
        cpu.f |= 0x80; // Set Sign flag for bit 7
    }
}

// Function to display CPU state
void display_cpu_state()
{
    std::cout << "A: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.a) << std::endl;
    std::cout << "F: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.f) << std::endl;
    std::cout << "B: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.b) << std::endl;
    std::cout << "C: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.c) << std::endl;
    std::cout << "D: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.d) << std::endl;
    std::cout << "E: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.e) << std::endl;
    std::cout << "H: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.h) << std::endl;
    std::cout << "L: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.l) << std::endl;
    std::cout << "I: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.i) << std::endl;
    std::cout << "R: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.r) << std::endl;
    std::cout << "A': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.a_prime) << std::endl;
    std::cout << "F': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.f_prime) << std::endl;
    std::cout << "B': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.b_prime) << std::endl;
    std::cout << "C': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.c_prime) << std::endl;
    std::cout << "D': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.d_prime) << std::endl;
    std::cout << "E': " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(cpu.e_prime) << std::endl;
    std::cout << "IFF1: " << static_cast<int>(cpu.interrupt_enable) << std::endl;
    std::cout << "IFF2: " << static_cast<int>(cpu.interrupt_enable) << std::endl;
    std::cout << "IM: 0" << std::endl;
    std::cout << "Hidden 16-bit math register: 00" << std::endl;
    std::cout << "IX: " << std::hex << std::setw(4) << std::setfill('0') << cpu.ix << std::endl;
    std::cout << "IY: " << std::hex << std::setw(4) << std::setfill('0') << cpu.iy << std::endl;
    std::cout << "PC: " << std::hex << std::setw(4) << std::setfill('0') << cpu.pc << std::endl;
    std::cout << "SP: " << std::hex << std::setw(4) << std::setfill('0') << cpu.sp << std::endl;
}

// Initialize the Z80
void z80_init()
{
    std::memset(&cpu, 0, sizeof(Z80_CPU));
    std::memset(ram, 0, sizeof(ram));
    cpu.pc = 0x0000; // Start execution at address 0x0001
    cpu.sp = 0x0000; // Initialize stack pointer to 0x0000
    cpu.ix = 0xFFFF; // Initialize IX register
    cpu.iy = 0xFFFF; // Initialize IY register
    cpu.r = 0x01;    // Initialize Refresh register
    cpu.f = 0x40;    // Set Zero flag
    std::cout << "Z80 initialized. PC: " << std::hex << std::setw(4) << std::setfill('0') << cpu.pc << ", SP: " << cpu.sp << std::endl;
}

// Memory functions
void z80_mem_write(uint16_t addr, uint8_t value)
{
    ram[addr] = value;
}

uint8_t z80_mem_read(uint16_t addr)
{
    return ram[addr];
}

// Execute Z80 instructions for the given number of cycles
int z80_execute(int cycles)
{
    int executed_cycles = 0;

    while (executed_cycles < cycles)
    {
        // Fetch instruction at the current PC
        uint8_t opcode = ram[cpu.pc];
        cpu.r = (cpu.r + 1) & 0x7F; // Increment refresh register
        std::cout << "Executing opcode: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(opcode) << " at PC: " << std::setw(4) << cpu.pc << std::endl;

        if (opcode == 0xCB)
        {
            uint8_t cb_opcode = ram[++cpu.pc];
            switch (cb_opcode)
            {

            case 0x2F:
            {
                uint16_t address = (cpu.h << 8) | cpu.l;
                uint8_t value = z80_mem_read(address);
                set_bit_flags(value, 5);
                executed_cycles += 12;
                std::cout << "BIT 5, (HL) at address: " << std::hex << address << std::endl;
                break;
            }

            case 0x27:
            {
                uint8_t carry = cpu.a & 0x01;          // LSB for carry
                cpu.a = (cpu.a >> 1) | (cpu.a & 0x80); // Preserve MSB
                cpu.f = carry ? 0x01 : 0x00;           // Set carry flag
                set_flags(cpu.a);                      // Update Zero and Sign flags
                executed_cycles += 8;                  // Correct cycle count
                std::cout << "SRA A (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
                break;
            }

            case 0x07:
            {
                uint8_t carry = (cpu.a & 0x80) >> 7;                        // MSB as carry
                cpu.a = (cpu.a << 1) | carry;                               // Rotate left
                cpu.f = (cpu.a == 0 ? 0x40 : 0x00) | (carry ? 0x01 : 0x00); // Set Zero and carry flags
                std::cout << "RLC A (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
                executed_cycles += 8;
                break;
            }

            case 0x0F:
            {
                uint8_t carry = cpu.a & 0x01;                               // LSB as carry
                cpu.a = (cpu.a >> 1) | (carry << 7);                        // Rotate right
                cpu.f = (cpu.a == 0 ? 0x40 : 0x00) | (carry ? 0x01 : 0x00); // Set Zero and carry flags
                std::cout << "RRC A (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
                executed_cycles += 8;
                break;
            }

            case 0x17:
            {
                uint8_t carry = (cpu.f & 0x01);                                 // Carry flag
                uint8_t new_carry = (cpu.a & 0x80) >> 7;                        // MSB as new carry
                cpu.a = (cpu.a << 1) | carry;                                   // Rotate left through carry
                cpu.f = (cpu.a == 0 ? 0x40 : 0x00) | (new_carry ? 0x01 : 0x00); // Set Zero and carry flags
                std::cout << "RL A (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
                executed_cycles += 8;
                break;
            }

            case 0x1F:
            {
                uint8_t carry = (cpu.f & 0x01);                                 // Carry flag
                uint8_t new_carry = cpu.a & 0x01;                               // LSB as new carry
                cpu.a = (cpu.a >> 1) | (carry << 7);                            // Rotate right through carry
                cpu.f = (cpu.a == 0 ? 0x40 : 0x00) | (new_carry ? 0x01 : 0x00); // Set Zero and carry flags
                std::cout << "RR A (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
                executed_cycles += 8;
                break;
            }

            case 0x08:
            {
                uint8_t carry = (cpu.b & 0x80) >> 7;                        // Extract MSB
                cpu.b = (cpu.b << 1) | carry;                               // Rotate left circular
                cpu.f = (cpu.b == 0 ? 0x40 : 0x00) | (carry ? 0x01 : 0x00); // Set flags
                std::cout << "RLC B (Result: " << std::hex << static_cast<int>(cpu.b) << ")" << std::endl;
                executed_cycles += 8; // Correct cycle count
                break;
            }

            case 0x28:
            {
                cpu.b = (cpu.b >> 1) | (cpu.b & 0x80);
                cpu.f = (cpu.b == 0 ? 0x40 : 0) | (cpu.b & 0x80); // Set Zero and Sign flags
                executed_cycles += 8;
                std::cout << "SRA B (Result: " << std::hex << static_cast<int>(cpu.b) << ")" << std::endl;
                break;
            }

            case 0x31:
            {
                cpu.f = (cpu.c & 0x01) ? 0x01 : 0; // Set carry flag
                cpu.c >>= 1;
                if (cpu.c == 0)
                    cpu.f |= 0x40; // Zero flag
                executed_cycles += 8;
                std::cout << "SRL C (Result: " << std::hex << static_cast<int>(cpu.c) << ")" << std::endl;
                break;
            }

            case 0x3A:
            {
                cpu.d = (cpu.d >> 1) | (cpu.d & 0x80);
                cpu.f = (cpu.d == 0 ? 0x40 : 0) | (cpu.d & 0x80); // Set Zero and Sign flags
                executed_cycles += 8;
                std::cout << "SRA D (Result: " << std::hex << static_cast<int>(cpu.d) << ")" << std::endl;
                break;
            }

            case 0x6F: // BIT 5, A
            {
                uint8_t mask = 1 << 5;                                     // Mask for bit 5
                cpu.f = (cpu.a & mask) ? (cpu.f & ~0x40) : (cpu.f | 0x40); // Set Zero flag if bit is 0
                cpu.f = (cpu.f & ~0x02) | 0x10;                            // Maintain unaffected flags (Half Carry set)
                std::cout << "BIT 5, A (Value: " << std::hex << static_cast<int>(cpu.a) << ", Result: "
                          << ((cpu.a & mask) ? "Set" : "Unset") << ")" << std::endl;
                executed_cycles += 8;
                break;
            }

            default:
                std::cout << "Unknown CB-prefixed opcode: " << std::hex << static_cast<int>(cb_opcode) << std::endl;
                executed_cycles += 4;
                break;
            }
            cpu.pc++;
            continue;
        }

        // Decode and execute the opcode
        switch (opcode)
        {
        case 0x00: // NOP
            std::cout << "NOP (No Operation)" << std::endl;
            cpu.pc++;
            executed_cycles += 4;
            break;

        case 0x02:
        {
            uint16_t address = (cpu.b << 8) | cpu.c;
            z80_mem_write(address, cpu.a);
            cpu.pc++;
            executed_cycles += 7;
            std::cout << "LD (BC), A (Address: " << std::hex << address << ")" << std::endl;
            break;
        }

        case 0x06:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.b = value;
            cpu.pc += 2;
            executed_cycles += 7;
            std::cout << "LD B, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0x76:
            std::cout << "HLT (Halt Execution)" << std::endl;
            executed_cycles += 4;
            std::cout << "Ran " << std::dec << executed_cycles << " cycles" << std::endl;
            return executed_cycles;

        case 0x3E:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.a = value;
            std::cout << "LD A, " << std::hex << static_cast<int>(value) << std::endl;
            cpu.pc += 2;
            executed_cycles += 7;
            break;
        }

        case 0x80:
        {
            uint16_t result = cpu.a + cpu.b;
            cpu.a = static_cast<uint8_t>(result);

            cpu.f = 0; // Reset flags
            if (cpu.a == 0)
                cpu.f |= 0x40; // Zero flag
            if (cpu.a & 0x80)
                cpu.f |= 0x80; // Sign flag
            if ((cpu.a ^ cpu.b ^ result) & 0x10)
                cpu.f |= 0x10; // Half-carry flag
            if (result > 0xFF)
                cpu.f |= 0x01; // carry flag

            std::cout << "ADD A, B (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
            cpu.pc++;
            executed_cycles += 4;
            break;
        }

        case 0xCD:
        {
            uint16_t target_address = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8); // Fetch 16-bit target address
            cpu.pc += 3;                                                        // Advance PC to the instruction after CALL
            ram[--cpu.sp] = (cpu.pc >> 8) & 0xFF;                               // Push high byte of PC to stack
            ram[--cpu.sp] = cpu.pc & 0xFF;                                      // Push low byte of PC to stack
            cpu.pc = target_address;                                            // Jump to target address
            executed_cycles += 17;                                              // CALL takes 17 cycles
            std::cout << "CALL to address: " << std::hex << target_address << std::endl;
            break;
        }

        case 0xC9:
        {
            uint16_t low_byte = ram[cpu.sp++];
            uint16_t high_byte = ram[cpu.sp++];
            cpu.pc = (high_byte << 8) | low_byte; // Pop the return address
            executed_cycles += 10;
            std::cout << "RET to address: " << std::hex << cpu.pc << std::endl;
            break;
        }

        case 0x2E:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.l = value;
            cpu.pc += 2;
            executed_cycles += 7;
            std::cout << "LD L, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0x01:
        {
            uint16_t value = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8);
            cpu.b = (value >> 8) & 0xFF;
            cpu.c = value & 0xFF;
            cpu.pc += 3;
            executed_cycles += 10;
            std::cout << "LD BC, " << std::hex << value << std::endl;
            break;
        }

        case 0xED:
        { // Extended instructions
            uint8_t ed_opcode = ram[++cpu.pc];
            switch (ed_opcode)
            {

            case 0x4F:
            {
                cpu.r = cpu.a;
                cpu.pc++;
                executed_cycles += 9;
                std::cout << "LD R, A" << std::endl;
                break;
            }

            case 0x5A:
            {
                uint32_t result = (cpu.h << 8 | cpu.l) + (cpu.d << 8 | cpu.e) + (cpu.f & 0x01);
                cpu.h = (result >> 8) & 0xFF;
                cpu.l = result & 0xFF;

                // Set flags
                cpu.f = 0;
                if (result & 0x10000)
                    cpu.f |= 0x01; // carry flag
                if ((cpu.h << 8 | cpu.l) == 0)
                    cpu.f |= 0x40; // Zero flag
                if (result & 0x8000)
                    cpu.f |= 0x80; // Sign flag
                if (((cpu.h << 8 | cpu.l) ^ (cpu.d << 8 | cpu.e) ^ result) & 0x1000)
                    cpu.f |= 0x10; // Half-carry flag

                executed_cycles += 15;
                std::cout << "ADC HL, DE (Result: " << std::hex << (cpu.h << 8 | cpu.l) << ")" << std::endl;
                cpu.pc++;
                break;
            }

            case 0x42:
            {
                uint32_t result = (cpu.h << 8 | cpu.l) - (cpu.b << 8 | cpu.c) - (cpu.f & 0x01);
                cpu.h = (result >> 8) & 0xFF;
                cpu.l = result & 0xFF;

                // Set flags
                cpu.f = 0x02; // Set Subtract flag
                if (result & 0x10000)
                    cpu.f |= 0x01; // carry flag
                if ((cpu.h << 8 | cpu.l) == 0)
                    cpu.f |= 0x40; // Zero flag
                if (result & 0x8000)
                    cpu.f |= 0x80; // Sign flag
                if (((cpu.h << 8 | cpu.l) ^ (cpu.b << 8 | cpu.c) ^ result) & 0x1000)
                    cpu.f |= 0x10; // Half-carry flag

                executed_cycles += 15;
                std::cout << "SBC HL, BC (Result: " << std::hex << (cpu.h << 8 | cpu.l) << ")" << std::endl;
                cpu.pc++;
                break;
            }

            default:
                std::cout << "Unknown ED-prefixed opcode: " << std::hex << static_cast<int>(ed_opcode) << std::endl;
                executed_cycles += 4;
                cpu.pc++;
                break;
            }
            break;
        }

        case 0xC3:
        {
            uint16_t target_address = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8);
            std::cout << "JP to address: " << std::hex << target_address << std::endl;
            cpu.pc = target_address; // Update PC to the target address
            executed_cycles += 10;
            break;
        }

        case 0x18:
        {
            int8_t offset = static_cast<int8_t>(ram[cpu.pc + 1]); // Signed 8-bit offset
            cpu.pc += 2;                                          // Move to next instruction
            cpu.pc += offset;                                     // Apply the relative jump
            executed_cycles += 12;
            std::cout << "JR (Jump Relative) by offset: " << std::dec << static_cast<int>(offset) << std::endl;
            break;
        }

        case 0x11:
        {
            uint16_t value = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8);
            cpu.d = (value >> 8) & 0xFF;
            cpu.e = value & 0xFF;
            cpu.pc += 3;
            executed_cycles += 10;
            std::cout << "LD DE, " << std::hex << value << std::endl;
            break;
        }

        case 0x21:
        {
            uint16_t value = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8);
            cpu.h = (value >> 8) & 0xFF;
            cpu.l = value & 0xFF;
            cpu.pc += 3;
            executed_cycles += 10;
            std::cout << "LD HL, " << std::hex << value << std::endl;
            break;
        }

        case 0x16:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.d = value;
            cpu.pc += 2; // Advance PC past the opcode and immediate value
            executed_cycles += 7;
            std::cout << "LD D, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0x26:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.h = value;
            cpu.pc += 2; // Advance PC past the opcode and immediate value
            executed_cycles += 7;
            std::cout << "LD H, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0x0E:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.c = value;
            cpu.pc += 2; // Advance PC past opcode and immediate value
            executed_cycles += 7;
            std::cout << "LD C, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0x1E:
        {
            uint8_t value = ram[cpu.pc + 1];
            cpu.e = value;
            cpu.pc += 2; // Advance PC past opcode and immediate value
            executed_cycles += 7;
            std::cout << "LD E, " << std::hex << static_cast<int>(value) << std::endl;
            break;
        }

        case 0xDD: // Prefix for IX operations
        case 0xFD:
        {                                                             // Prefix for IY operations
            uint16_t &index_reg = (opcode == 0xDD) ? cpu.ix : cpu.iy; // Select IX or IY
            uint8_t next_opcode = ram[++cpu.pc];                      // Fetch the next opcode

            if (next_opcode == 0xCB)
            {
                int8_t offset = static_cast<int8_t>(ram[++cpu.pc]); // Signed offset
                uint8_t cb_opcode = ram[++cpu.pc];                  // Fetch CB-prefixed opcode

                switch (cb_opcode)
                {
                case 0x6E:
                {
                    uint16_t address = index_reg + offset; // Calculate address
                    uint8_t value = z80_mem_read(address); // Read from memory
                    set_bit_flags(value, 5);               // Test bit 5
                    executed_cycles += 20;
                    std::cout << "BIT 5, (" << ((opcode == 0xDD) ? "IX" : "IY") << "+" << std::hex << static_cast<int>(offset) << ") (Address: " << address << ")" << std::endl;
                    break;
                }
                default:
                    std::cout << "Unknown CB-prefixed opcode after " << ((opcode == 0xDD) ? "DD" : "FD") << ": " << std::hex << static_cast<int>(cb_opcode) << std::endl;
                    executed_cycles += 4;
                    break;
                }
            }
            else
            {

                switch (next_opcode)
                {
                case 0x21:
                {
                    uint16_t value = ram[cpu.pc + 1] | (ram[cpu.pc + 2] << 8);
                    index_reg = value;
                    cpu.pc += 3;
                    executed_cycles += 14;
                    std::cout << "LD " << ((opcode == 0xDD) ? "IX" : "IY") << ", " << std::hex << value << std::endl;
                    break;
                }

                case 0x77:
                {
                    int8_t offset = static_cast<int8_t>(ram[++cpu.pc]);
                    uint16_t address = index_reg + offset;
                    z80_mem_write(address, cpu.a);
                    cpu.pc++;
                    executed_cycles += 19;
                    std::cout << "LD (" << ((opcode == 0xDD) ? "IX" : "IY") << "+" << std::hex << static_cast<int>(offset) << "), A (Address: " << address << ")" << std::endl;
                    break;
                }

                case 0x96:
                {
                    int8_t offset = static_cast<int8_t>(ram[++cpu.pc]);
                    uint16_t address = index_reg + offset;
                    uint8_t value = z80_mem_read(address);
                    uint16_t result = cpu.a - value;
                    cpu.f = 0; // Reset flags
                    if (result == 0)
                        cpu.f |= 0x40; // Zero flag
                    if (result & 0x80)
                        cpu.f |= 0x80; // Sign flag
                    if ((cpu.a ^ value ^ result) & 0x10)
                        cpu.f |= 0x10; // Half-carry flag
                    if (result > 0xFF)
                        cpu.f |= 0x01; // Carry flag
                    cpu.a = static_cast<uint8_t>(result);

                    cpu.pc++;
                    executed_cycles += 19;
                    std::cout << "SUB (" << ((opcode == 0xDD) ? "IX" : "IY") << "+" << std::hex << static_cast<int>(offset) << ") (Address: " << address << ")" << std::endl;
                    break;
                }

                default:
                    std::cout << "Unknown opcode after prefix: " << std::hex << static_cast<int>(next_opcode) << std::endl;
                    cpu.pc++;
                    executed_cycles += 4;
                    break;
                }
            }
            break;
        }

        case 0x90:
        {
            uint16_t result = cpu.a - cpu.b; // Perform subtraction
            cpu.f = 0;                       // Reset flags
            if (result == 0)
                cpu.f |= 0x40; // Zero flag
            if (result & 0x80)
                cpu.f |= 0x80; // Sign flag
            if ((cpu.a ^ cpu.b ^ result) & 0x10)
                cpu.f |= 0x10; // Half-carry flag
            if (cpu.a < cpu.b)
                cpu.f |= 0x01;                    // Carry flag
            cpu.a = static_cast<uint8_t>(result); // Store the result
            cpu.pc++;
            executed_cycles += 4;
            std::cout << "SUB A, B (Result: " << std::hex << static_cast<int>(cpu.a) << ")" << std::endl;
            break;
        }

        case 0x37:
        {
            cpu.f &= ~(0x10); // Clear H flag
            cpu.f |= 0x01;    // Set Carry flag
            cpu.f &= ~(0x02); // Clear N flag

            std::cout << "SCF (Set Carry Flag)" << std::endl;
            cpu.pc++;
            executed_cycles += 4;
            break;
        }

        default:
            std::cout << "Unknown opcode: " << std::hex << static_cast<int>(opcode) << std::endl;
            cpu.pc++;
            executed_cycles += 4; // Default cycle count for unimplemented instructions
            break;
        }

        // Break if PC goes out of bounds
        if (cpu.pc >= 65536)
        {
            std::cout << "PC out of bounds. Halting execution." << std::endl;
            break;
        }
    }

    std::cout << "Ran " << executed_cycles << " cycles" << std::endl;
    return executed_cycles;
}

void z80_mem_load(const std::string &filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
    {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }

    std::streamsize size = file.tellg();
    if (size > 65536)
    {
        std::cerr << "Error: File too large to fit in memory." << std::endl;
        file.close();
        return;
    }

    file.seekg(0, std::ios::beg);
    file.read(reinterpret_cast<char *>(ram), size);
    if (!file)
    {
        std::cerr << "Error: Failed to read file " << filename << std::endl;
    }
    else
    {
        std::cout << "Loaded binary file: " << filename << " (Size: " << size << " bytes)" << std::endl;
    }

    file.close();
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " program.bin" << std::endl;
        return -1;
    }

    z80_init();
    z80_mem_load(argv[1]);

    int cycles = z80_execute(NUM_CYCLES);
    display_cpu_state();

    return 0;
}

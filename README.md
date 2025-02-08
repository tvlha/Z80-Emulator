# Z80 Emulator Project
**CMSC 411: Computer Architecture – Fall 2024**  
**Author: Talha Waseem**  
**Professor: Dr. Sebald**

This project implements a Z80 microprocessor emulator in C++, replicating key functionalities of the Z80 CPU.

## Project Features
- **Core Instruction Set**: Supports key Z80 operations.
- **Special Prefixes**: Handles `CB`, `DD`, `FD`, and `ED` instructions.
- **Memory Emulation**: Includes 64KB RAM emulation.

## Compilation
To compile the project, use:
```bash
g++ -o z80_emulator z80_emulator.cpp
```

## Execution
To test the code, use:
```bash
./z80_emulator <file>.bin
```

## Testing
This project was tested using the .bin files provided by the professor. They've been included in the 'Testing' directory.

## Resources
1. **[How to Write a Computer Emulator](http://www.emulation.org/EMUL8/HOWTO.html)**
2. **[Z80 Family CPU Manual](http://www.zilog.com/docs/z80/z80cpu_um.pdf)**
3. **[Computer Emulation Resources](https://fms.komkon.org/EMUL8/)**
4. **[Z80.info](http://z80.info/)**
5. **[C++ Reference Library](https://en.cppreference.com/w/)**
6. **[Stack Overflow](https://stackoverflow.com/)**
7. **[Official Project Outline](https://userpages.cs.umbc.edu/lsebald1/cmsc411-fa2024/project.shtml)**

## Known Issues
Here's the fun part, this is going to be a long list.
> **Note**: This list only refers to known issues as far as the testing files provided by the professor.
1. **basic.bin**
  - R is 02 instead of 01
  - PC is 0000 instead of 0001
2. **bin.bit**
  - F is 10 instead of 40
  - PC is 0004 instead of 0005
3. **call-return.bin**
  - PC is 0003 instead of 0004
4. **divide-8.bin**
  - A is b2 instead of f8
  - F is 80 instead of a8
  - R is 08 instead of 0a
  - PC is 000b instead of 000c
  - Cycle count is 46 instead of 58
5. **jump.bin**
  - R is 06 instead of 05
  - PC is 000a instead of 000b
6. **jump-relative.bin**
  - R is 06 instead of 05
  - PC is 0009 instead of 000a
7. **load-regs.bin**
  - R is 09 instead of 08
  - PC is 000e instead of 000f
8. **multiply-4.bin**
  - A is 04 instead of 2a
  - F is 00 instead of 28
  - R is 07 instead of 08
  - PC is 0009 instead of 000a
9. **offset-add.bin**
  - A is 00 instead of 02
  - F is 40 instead of 02
  - R is 0c instead of 12
  - PC is 001b instead of 001c
10. **offset-bit.bin**
  - R is 08 instead of 0b
  - PC is 0011 instead of 0012
  - Cycle count is 82 instead of 78
11. **rotate.bin**
  - F is 01 instead of 84
  - B is 40 instead of 10
  - C is 20 instead of 41
  - D is 20 instead of 90
  - R is 0e instead of 11
  - PC is 0014 instead of 0015
  - Cycle count is 72 instead of 80
12. **shift.bin**
  - A is 10 instead of 40
  - C is 10 instead of 41
  - R is 0a instead of 0d
  - PC is 0010 instead of 0011
13. **simple-add.bin**
  - F is 00 instead of 28
  - R is 04 instead of 05
  - PC is 0005 instead of 0006
14. **simple-sub.bin**
  - F is 00 instead of 22
  - R is 05 instead of 04
  - PC is 0005 instead of 0006
15. **sixteen-bit.bin**
  - F is 12 instead of 02
  - R is 0a instead of 0b
  - PC is 0010 instead of 0011

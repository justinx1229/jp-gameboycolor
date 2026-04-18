#ifndef MEM 
#define MEM 

#include <cstdint>
#include <iostream>

const uint32_t SIZE_ROM_BANK = 1 << 14;
const uint32_t SIZE_VRAM = 1 << 13;
const uint32_t SIZE_WRAM = 1 << 12;
const uint32_t SIZE_OAM = 160;
const uint32_t SIZE_REGS = 128;
const uint32_t SIZE_HRAM = 175;

// 1100000000000000
const uint16_t RB0_MASK = (1 << 14) | (1 << 15);
const uint16_t LO_14 = (1 << 14) - 1;

uint8_t read_byte(uint16_t address);

void write_byte(uint16_t address, uint8_t byte);

#endif
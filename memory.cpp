#include "memory.h"

uint8_t ROM_bank_00[SIZE_ROM_BANK];
uint8_t ROM_bank_01_NN[SIZE_ROM_BANK];

uint8_t VRAM[SIZE_VRAM];
uint8_t ext_RAM[SIZE_VRAM];

uint8_t WRAM_1[SIZE_WRAM];
uint8_t WRAM_2[SIZE_WRAM];

uint8_t OAM[SIZE_OAM];

uint8_t regs[SIZE_REGS];
uint8_t HRAM[SIZE_HRAM];

uint8_t read_byte(uint16_t address) {
    return 0;
}

void write_byte(uint16_t address, uint8_t byte) {
    // neither the 14th nor 15th bits should be set (gives ROM bank 00)
    if (!(address & RB0_MASK)) {
        ROM_bank_00[address] = byte;
    }
    // 14th bit is set, 15th bit should not be set (gives switchable ROM bank)
    else if ((address & (1 << 14)) && !(address & (1 << 15))) {
        ROM_bank_01_NN[address & LO_14] = byte;
    }
}
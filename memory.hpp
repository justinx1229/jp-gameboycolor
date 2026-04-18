#include <cstdint>

const uint32_t SIZE_ROM_BANK = 1 << 14;
const uint32_t SIZE_VRAM = 1 << 13;
const uint32_t SIZE_WRAM = 1 << 12;
const uint32_t SIZE_OAM = 160;
const uint32_t SIZE_REGS = 128;
const uint32_t SIZE_HRAM = 175;

uint8_t ROM_bank_00[SIZE_ROM_BANK];
uint8_t ROM_bank_01_NN[SIZE_ROM_BANK];

uint8_t VRAM[SIZE_VRAM];
uint8_t ext_RAM[SIZE_VRAM];

uint8_t WRAM_1[SIZE_WRAM];
uint8_t WRAM_2[SIZE_WRAM];

uint8_t OAM[SIZE_OAM];

uint8_t regs[SIZE_REGS];
uint8_t HRAM[SIZE_HRAM];


uint8_t read_byte(uint16_t address);

void write_byte(uint16_t address, uint8_t byte);
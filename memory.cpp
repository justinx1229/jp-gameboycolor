
#include "memory.h"
#include "joypad.h"
#include "timer.h"
#include <cstring>

uint8_t ROM_bank_00[SIZE_ROM_BANK];
uint8_t ROM_bank_01_NN[SIZE_ROM_BANK];

uint8_t VRAM[SIZE_VRAM][2];
uint8_t ext_RAM[SIZE_VRAM];

uint8_t WRAM_1[SIZE_WRAM];
uint8_t WRAM_2[SIZE_WRAM];

uint8_t OAM[SIZE_OAM];

uint8_t regs[SIZE_REGS];
uint8_t HRAM[SIZE_HRAM];
uint8_t IE;
uint8_t ly; 
uint8_t lyc; 

bool vram_bank = false;
bool oam_done = false;
bool run_done = false;

void reset_memory() {
    memset(ROM_bank_00, 0, sizeof(ROM_bank_00));
    memset(ROM_bank_01_NN, 0, sizeof(ROM_bank_01_NN));
    memset(VRAM, 0, sizeof(VRAM));
    memset(ext_RAM, 0, sizeof(ext_RAM));
    memset(WRAM_1, 0, sizeof(WRAM_1));
    memset(WRAM_2, 0, sizeof(WRAM_2));
    memset(OAM, 0, sizeof(OAM));
    memset(regs, 0, sizeof(regs));
    memset(HRAM, 0, sizeof(HRAM));
    IE = 0;
    reset_joypad();
    reset_timer();
}

uint8_t read_vram(uint16_t address, bool bank) {
    return VRAM[address - 0x8000][bank];
}

uint8_t read_byte(uint16_t address) {
    if (address < 0x4000) {
        return ROM_bank_00[address];
    }
    else if (address < 0x8000) {
        return ROM_bank_01_NN[address & LO_14];
    }
    else if (address < 0xA000) {
        return VRAM[address - 0x8000][vram_bank];
    }
    else if (address < 0xC000) {
        return ext_RAM[address - 0xA000];
    }
    else if (address < 0xD000) {
        return WRAM_1[address - 0xC000];
    }
    else if (address < 0xE000) {
        return WRAM_2[address - 0xD000];
    }

    // prohibited area, but should mirror 0xC000-0xCFFF
    else if (address < 0xF000) {
        return WRAM_1[address - 0xE000];
    }
    else if (address < 0xFE00) {
        return WRAM_2[address - 0xF000];
    }
    
    else if (address < 0xFEA0) {
        return OAM[address - 0xFE00];
    }
    else if (address >= 0xFF00 && address < 0xFF80) {
        // Blargg tests print to the serial port by writing a byte to SB and then writing 0x81 to SC.
        // idk how this works lowkey but it does so here we are
        switch (address) {
            case 0xFF00: {
                return read_joypad();
                break;
            }
            case 0xFF4F: {
                return vram_bank;
                break;
            } 
            case 0xFF44: {
                return ly;
                break;
            }
            case 0xFF45: {
                return lyc;
                break;
            }
        }
        
        return regs[address - 0xFF00];
        
    }
    else if (address < 0xFFFF) {
        return HRAM[address - 0xFF80];
    }
    else if (address == 0xFFFF) {
        return IE;
    }

    return 0;
}

void write_byte(uint16_t address, uint8_t byte) {
    if (address < 0x4000) {
        ROM_bank_00[address] = byte;
    }
    else if (address < 0x8000) {
        ROM_bank_01_NN[address & LO_14] = byte;
    }
    else if (address < 0xA000) {
        VRAM[address - 0x8000][vram_bank] = byte;
    }
    else if (address < 0xC000) {
        ext_RAM[address - 0xA000] = byte;
    }
    else if (address < 0xD000) {
        WRAM_1[address - 0xC000] = byte;
    }
    else if (address < 0xE000) {
        WRAM_2[address - 0xD000] = byte;
    }
    else if (address < 0xF000) {
        WRAM_1[address - 0xE000] = byte;
    }
    else if (address < 0xFE00) {
        WRAM_2[address - 0xF000] = byte;
    }
    else if (address < 0xFEA0) {
        if (oam_done) {
            OAM[address - 0xFE00] = byte;
        }
    }
    else if (address >= 0xFF00 && address < 0xFF80) {
        regs[address - 0xFF00] = byte;
        timer_write(address, byte);

        // Blargg tests print to the serial port by writing a byte to SB and then writing 0x81 to SC.
        // idk how this works lowkey but it does so here we are
        switch (address) {
            case 0xFF00: {
                write_joypad(byte);
                break;
            }
            case 0xFF02: {
                if (address == 0xFF02 && byte == 0x81) {
                    std::cout << static_cast<char>(regs[0x01]) << std::flush;
                    regs[0x02] = 0;
                }
                break;
            }
            case 0xFF4F: {
                vram_bank = byte & 1;
                break;
            } 
            case 0xFF44: {
                ly = byte;
                break;
            }
            case 0xFF45: {
                lyc = byte;
                break;
            }
        }
    }
    else if (address < 0xFFFF) {
        HRAM[address - 0xFF80] = byte;
    }
    else if (address == 0xFFFF) {
        IE = byte;
    }
}

void write16(uint16_t address, uint16_t val) {
    write_byte(address, val & LO_8);
    write_byte(address + 1, val >> 8);
}

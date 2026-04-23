
#include "memory.h"
#include "joypad.h"
#include "timer.h"
#include <cstring>
#include <vector>

uint8_t ROM_bank_00[SIZE_ROM_BANK];
uint8_t ROM_bank_01_NN[SIZE_ROM_BANK];

uint8_t VRAM[SIZE_VRAM][2];
uint8_t ext_RAM[SIZE_EXT_RAM];

uint8_t WRAM_1[SIZE_WRAM];
uint8_t WRAM_2[SIZE_WRAM];

uint8_t OAM[SIZE_OAM];

uint8_t CRAM[SIZE_CRAM];

uint8_t regs[SIZE_REGS];
uint8_t HRAM[SIZE_HRAM];
uint8_t IE;
uint8_t ly; 
uint8_t lyc; 

bool vram_bank = false;
bool oam_done = true;
bool run_done = true;
bool window = false;

std::vector<uint8_t> cartridge_rom;
uint8_t cartridge_type = 0;
uint32_t rom_bank_count = 0;
bool ram_enabled = false;
uint8_t mbc3_rom_bank = 1;
uint8_t mbc3_ram_bank = 0;

uint32_t effective_rom_bank(uint16_t address) {
    if (!rom_bank_count) {
        return 0;
    }

    if (cartridge_type == 0x11 || cartridge_type == 0x12 || cartridge_type == 0x13) {
        if (address < 0x4000) {
            return 0;
        }

        uint32_t bank = mbc3_rom_bank & 0x7F;
        bank %= rom_bank_count;
        if (!bank) {
            bank = 1 % rom_bank_count;
        }
        return bank;
    }

    if (address < 0x4000) {
        return 0;
    }

    if (rom_bank_count <= 1) {
        return 0;
    }

    return 1;
}

void reset_memory() {
    memset(VRAM, 0, sizeof(VRAM));
    memset(ext_RAM, 0, sizeof(ext_RAM));
    memset(WRAM_1, 0, sizeof(WRAM_1));
    memset(WRAM_2, 0, sizeof(WRAM_2));
    memset(OAM, 0, sizeof(OAM));
    memset(regs, 0, sizeof(regs));
    memset(HRAM, 0, sizeof(HRAM));
    IE = 0;
    cartridge_rom.clear();
    cartridge_type = 0;
    rom_bank_count = 0;
    ram_enabled = false;
    mbc3_rom_bank = 1;
    mbc3_ram_bank = 0;
    reset_joypad();
    reset_timer();
}

void load_rom(const std::vector<uint8_t> &bytes) {
    cartridge_rom = bytes;
    rom_bank_count = (bytes.size() + SIZE_ROM_BANK - 1) / SIZE_ROM_BANK;
    cartridge_type = bytes.size() > 0x147 ? bytes[0x147] : 0;
    ram_enabled = false;
    mbc3_rom_bank = 1;
    mbc3_ram_bank = 0;

    memset(ROM_bank_00, 0, sizeof(ROM_bank_00));
    memset(ROM_bank_01_NN, 0, sizeof(ROM_bank_01_NN));

    uint32_t first_bank_size = std::min<uint32_t>(SIZE_ROM_BANK, bytes.size());
    memcpy(ROM_bank_00, bytes.data(), first_bank_size);

    if (bytes.size() > SIZE_ROM_BANK) {
        uint32_t second_bank_size = std::min<uint32_t>(SIZE_ROM_BANK, bytes.size() - SIZE_ROM_BANK);
        memcpy(ROM_bank_01_NN, bytes.data() + SIZE_ROM_BANK, second_bank_size);
    }
}

uint8_t read_vram(uint16_t address, bool bank) {
    return VRAM[address - 0x8000][bank];
}

uint8_t read_byte(uint16_t address) {
    if (address < 0x4000) {
        if (cartridge_rom.empty()) {
            return ROM_bank_00[address];
        }

        uint32_t bank = effective_rom_bank(address);
        uint32_t index = bank * SIZE_ROM_BANK + address;
        return index < cartridge_rom.size() ? cartridge_rom[index] : 0xFF;
    }
    else if (address < 0x8000) {
        if (cartridge_rom.empty()) {
            return ROM_bank_01_NN[address & LO_14];
        }

        uint32_t bank = effective_rom_bank(address);
        uint32_t index = bank * SIZE_ROM_BANK + (address & LO_14);
        return index < cartridge_rom.size() ? cartridge_rom[index] : 0xFF;
    }
    else if (address < 0xA000) {
    
        return VRAM[address - 0x8000][vram_bank & cgb_mode];
    }
    else if (address < 0xC000) {
        if (!ram_enabled) {
            return 0xFF;
        }

        uint32_t ram_index = mbc3_ram_bank * SIZE_EXT_RAM_BANK + (address - 0xA000);
        return ram_index < SIZE_EXT_RAM ? ext_RAM[ram_index] : 0xFF;
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
        if (cartridge_type == 0x11 || cartridge_type == 0x12 || cartridge_type == 0x13) {
            if (address < 0x2000) {
                ram_enabled = (byte & 0x0F) == 0x0A;
            }
            else {
                mbc3_rom_bank = byte & 0x7F;
                if (!mbc3_rom_bank) {
                    mbc3_rom_bank = 1;
                }
            }
        }
        else {
            ROM_bank_00[address] = byte;
        }
    }
    else if (address < 0x8000) {
        if (cartridge_type == 0x11 || cartridge_type == 0x12 || cartridge_type == 0x13) {
            if (address < 0x6000) {
                mbc3_ram_bank = byte & LO_2;
            }
        }
        else {
            ROM_bank_01_NN[address & LO_14] = byte;
        }
    }
    else if (address < 0xA000) {
        // if (mode != Mode::DRAW) {
            VRAM[address - 0x8000][vram_bank && cgb_mode] = byte;
        // }
    }
    else if (address < 0xC000) {
        if (ram_enabled) {
            uint32_t ram_index = mbc3_ram_bank * SIZE_EXT_RAM_BANK + (address - 0xA000);
            if (ram_index < SIZE_EXT_RAM) {
                ext_RAM[ram_index] = byte;
            }
        }
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
        // if (mode != Mode::OAM && mode != Mode::DRAW) {
        // std::cout << "write OAM" << address << " " << (uint32_t)byte << "\n";
            OAM[address - 0xFE00] = byte;
        // }
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
            case 0xFF40: {
                if (!(byte & (1 << 5))) {
                    window = false;
                }
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
            case 0xFF46: {
                uint16_t source = ((uint16_t)byte) << 8;
                for (uint16_t i = 0; i < SIZE_OAM; i++) {
                    OAM[i] = read_byte(source + i);
                }
                break;
            }
            case 0xFF69: {
                if (mode != Mode::DRAW) {
                    CRAM[regs[0xFF68 - 0xFF00] & LO_6] = byte;
                }
                regs[0xFF68 - 0xFF00] = (regs[0xFF68 - 0xFF00] + (regs[0xFF68 - 0xFF00] >> 7)) & (LO_8 ^ (1 << 6));
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

uint8_t read_cram(uint8_t address) {
    return CRAM[address];
}

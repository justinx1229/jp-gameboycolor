
#pragma once

#include "consts.h"

#include <cstdint>
#include <iostream>

const uint32_t SIZE_ROM_BANK = 1 << 14;
const uint32_t SIZE_VRAM = 1 << 13;
const uint32_t SIZE_WRAM = 1 << 12;
const uint32_t SIZE_OAM = 160;
const uint32_t SIZE_REGS = 128;
const uint32_t SIZE_HRAM = 127;

// 1100000000000000
const uint16_t RB0_MASK = (1 << 14) | (1 << 15);

uint8_t read_byte(uint16_t address);
extern uint8_t ly;
extern uint8_t lyc;
extern bool oam_done;
extern bool run_done;

void write_byte(uint16_t address, uint8_t byte);

void write16(uint16_t address, uint16_t val);

void reset_memory();

uint8_t read_vram(uint16_t address, bool bank);

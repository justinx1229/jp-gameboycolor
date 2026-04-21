
#pragma once

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <bitset>
#include <chrono>

const uint32_t WIDTH = 160;
const uint32_t HEIGHT = 144;

const uint32_t FPS = 60;
const double FRAME_LEN = 1000 / FPS;
const uint32_t CYC = 17476;

const uint8_t CB = 203; 
const uint16_t LO_16 = (1 << 16) - 1;
const uint16_t LO_14 = (1 << 14) - 1;
const uint16_t LO_12 = (1 << 12) - 1;
const uint8_t LO_8 = 255;
const uint16_t HI_8 = LO_8 << 8;
const uint8_t LO_7 = 127;
const uint8_t LO_6 = 63;
const uint8_t LO_5 = 31;
const uint8_t LO_4 = 15;
const uint8_t LO_3 = 7;
const uint8_t LO_2 = 3;

const uint8_t I_JUMPS[5] = {0x40, 0x48, 0x50, 0x58, 0x60};
const uint32_t GB_COLOR[4] = {0xFFFFFF, 0xD3D3D3, 0xA9A9A9, 0x000000};

extern uint8_t cgb_mode;


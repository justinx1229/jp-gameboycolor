#ifndef CONSTS 
#define CONSTS

#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include <string>
#include <bitset>

const uint32_t WIDTH = 160;
const uint32_t HEIGHT = 144;

const uint32_t FPS = 60;
const uint32_t FRAME_LEN = 1000 / FPS;

const uint8_t CB = 203; 
const uint16_t LO_16 = (1 << 16) - 1;
const uint16_t LO_14 = (1 << 14) - 1;
const uint16_t LO_12 = (1 << 12) - 1;
const uint8_t LO_8 = 255;
const uint16_t HI_8 = LO_8 << 8;
const uint8_t LO_4 = 15;
const uint8_t LO_3 = 7;
const uint8_t LO_2 = 3;

#endif
#pragma once

#include "memory.h"
#include "consts.h"
#include <vector>
#include <array>
#include <algorithm>

extern uint32_t frame_buffer[HEIGHT][WIDTH];

void run_ppu(uint32_t m_cycles);

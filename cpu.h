
#pragma once

#include "consts.h"

#include <iostream>
#include <bitset>
#include <string>
#include <cstdint>

extern uint8_t flags[4];

void reset_cpu();

uint8_t run();

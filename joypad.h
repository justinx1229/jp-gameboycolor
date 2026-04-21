#pragma once

#include <cstdint>

enum class Button {
    RIGHT,
    LEFT,
    UP,
    DOWN,
    A,
    B,
    SELECT,
    START
};

void reset_joypad();
uint8_t read_joypad();
void write_joypad(uint8_t byte);
bool set_button(Button button, bool pressed);

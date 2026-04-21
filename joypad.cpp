#include "joypad.h"

namespace {
uint8_t joypad_select = 0x30;
bool buttons[8] = {};

uint8_t button_index(Button button) {
    return static_cast<uint8_t>(button);
}
}

void reset_joypad() {
    joypad_select = 0x30;
    for (bool &button : buttons) {
        button = false;
    }
}

uint8_t read_joypad() {
    uint8_t value = 0xCF | joypad_select;

    if (!(joypad_select & (1 << 4))) {
        if (buttons[button_index(Button::RIGHT)]) {
            value &= ~1;
        }
        if (buttons[button_index(Button::LEFT)]) {
            value &= ~(1 << 1);
        }
        if (buttons[button_index(Button::UP)]) {
            value &= ~(1 << 2);
        }
        if (buttons[button_index(Button::DOWN)]) {
            value &= ~(1 << 3);
        }
    }

    if (!(joypad_select & (1 << 5))) {
        if (buttons[button_index(Button::A)]) {
            value &= ~1;
        }
        if (buttons[button_index(Button::B)]) {
            value &= ~(1 << 1);
        }
        if (buttons[button_index(Button::SELECT)]) {
            value &= ~(1 << 2);
        }
        if (buttons[button_index(Button::START)]) {
            value &= ~(1 << 3);
        }
    }

    return value;
}

void write_joypad(uint8_t byte) {
    joypad_select = byte & 0x30;
}

bool set_button(Button button, bool pressed) {
    uint8_t index = button_index(button);
    bool newly_pressed = pressed && !buttons[index];
    buttons[index] = pressed;
    return newly_pressed;
}

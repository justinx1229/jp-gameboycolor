#include "ppu.h"

uint32_t dots = 0;

enum class Mode {
    OAM,
    DRAW,
    HBLANK,
    VBLANK
};

Mode mode = Mode::OAM;

std::vector<std::array<uint8_t, 4>> sprites; 
uint8_t lcdc;
uint32_t frame_buffer[WIDTH][HEIGHT];

void f_lyc() {
    if (ly == lyc) {
        write_byte(0xFF41, read_byte(0xFF41) | (1 << 2));
        if (read_byte(0xFF41) & (1 << 6)) {
            write_byte(0xFF0F, read_byte(0xFF0F) | (1 << 1));
        }
    }   
}

void compute_mode() {
    if (ly < 144) {
        if (dots <= 80) {
            mode = Mode::OAM;
        }
        else if (dots <= 252) {
            mode = Mode::DRAW;
        }
        else {
            mode = Mode::HBLANK;
        }
    }

    if (ly == 144) {
        mode = Mode::VBLANK;
        // vblank interrupt
        write_byte(0xFF0F, read_byte(0xFF0F) | 1);
    }
    else if (ly > 153) {
        mode = Mode::OAM;
        ly = 0;
        f_lyc();
    }
}

void run_oam() {
    if (oam_done) {
        return;
    }
    if (read_byte(0xFF41) & (1 << 5)) {
        write_byte(0xFF0F, read_byte(0xFF0F) & (1 << 1));
    }

    uint8_t height = ((read_byte(0xFF40) & (1 << 2)) ? 16 : 8);
    
    for (uint32_t i = 0xFE00; i < 0xFE9F; i += 4) {
        uint8_t y_pos = read_byte(i);
        uint8_t x_pos = read_byte(i + 1);
        uint8_t tile_index = read_byte(i + 2);
        uint8_t attributes = read_byte(i + 3);

        if (ly + 16 >= y_pos && ly + 16 < y_pos + height) {
            sprites.push_back({y_pos, x_pos, tile_index, attributes});
        }

        if (sprites.size() == 10) {
            break;
        }
    }

    oam_done = true;
    run_done = false;
}

std::array<uint8_t, 17> get_tile_data(uint8_t tv) {
    uint16_t base;
    if (lcdc & (1 << 3)) {
        base = 0x9C00;
    }
    else {
        base = 0x9800;
    }

    uint8_t tile_id = read_byte(base + tv);

    uint16_t start_index;

    if (lcdc & (1 << 4)) {
        start_index = 0x8000 + (((uint16_t)tile_id) << 4);
    }
    else {
        start_index = 0x9000 + (((uint16_t)((int8_t)tile_id)) << 4);
    }

    std::array<uint8_t, 17> data;
    for (uint32_t i = 0; i < 16; i++) {
        data[i] = read_vram(start_index + i, 0);
    }

    // get attribute
    if (cgb_mode) {
        data[16] = read_vram(base + tv, 1);
    }

    return data; 
}

void draw_bg() {
    uint8_t scx = read_byte(0xFF42);
    uint8_t scy = read_byte(0xFF43);

    for (uint8_t i = 0; i < WIDTH; i++) {
        uint8_t x = ly + scy;
        uint8_t y = i + scx;
        x /= 8; y /= 8;
        uint8_t tile_id = x * 32 + y;

        std::array<uint8_t, 17> tile_data = get_tile_data(tile_id);

        if (cgb_mode) {
            uint8_t attribute = tile_data[16];
        }
        else {
            uint8_t row = x % 8;
            uint8_t col = 7 - (y % 8);

            uint8_t color = (((tile_data[2 * row] >> col) & 1) << 1) | ((tile_data[2 * row + 1] >> col) & 1);
            frame_buffer[ly][i] = color;
        }

        
    }
}

void run_draw() {
    if (run_done) {
        return;
    }


    lcdc = read_byte(0xFF40);

    if (!cgb_mode || lcdc & 1) {
        draw_bg();
    }
}


void run_ppu(uint32_t m_cycles) {
    uint32_t t_cycles = 4 * m_cycles;
    // add dots
    dots += t_cycles;
    
    // if dots is too big, start over. 
    if (dots >= 456) {
        dots -= 456;
        oam_done = false;
        ly++;
        f_lyc(); 
    }

    compute_mode();

    uint8_t value;
    switch (mode) {
        case Mode::OAM: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)) | 2);
            run_oam();
            break;
        }
        case Mode::DRAW: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)) | 3);
            // run_draw();
            break;
        }
        case Mode::HBLANK: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)));
            // run_hblank();
            break;
        }
        case Mode::VBLANK: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)) | 1);
            // run_vblank();
            break;
        }
    }
    
}
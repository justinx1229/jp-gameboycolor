#include "ppu.h"

uint32_t dots = 0;

Mode mode = Mode::VBLANK;

std::vector<std::array<uint8_t, 4>> sprites; 
uint8_t lcdc;
uint32_t frame_buffer[HEIGHT][WIDTH];
bool prio[HEIGHT][WIDTH];

bool good_print = false;
bool should_print = false;

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
            if (mode == Mode::VBLANK) {
                if (read_byte(0xFF41) & (1 << 5)) {
                    write_byte(0xFF0F, read_byte(0xFF0F) & (1 << 1));
                }
            }
            mode = Mode::OAM;
        }
        else if (dots <= 252) {
            mode = Mode::DRAW;
        }
        else {
            if (mode == Mode::DRAW) {
                // hblank interrupt
                if (read_byte(0xFF41) & (1 << 3)) {
                    write_byte(0xFF0F, read_byte(0xFF0F) | 2);
                } 
            }
            mode = Mode::HBLANK;
        }
    }

    if (ly == 144) {
        mode = Mode::VBLANK;
        window = false;
        // vblank interrupt
        write_byte(0xFF0F, read_byte(0xFF0F) | 1);
        if (read_byte(0xFF41) & (1 << 4)) {
            write_byte(0xFF0F, read_byte(0xFF0F) | 2);
        } 
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

    uint8_t height = ((read_byte(0xFF40) & (1 << 2)) ? 16 : 8);
    
    for (uint32_t i = 0xFE00; i < 0xFE9F; i += 4) {
        uint8_t y_pos = read_byte(i);
        uint8_t x_pos = read_byte(i + 1);
        // std::cout << i << " " << (uint32_t)x_pos << " " << (uint32_t)y_pos << "\n";
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

std::array<uint8_t, 17> get_tile_data(uint32_t tv, bool is_window) {
    uint16_t base;
    if (!is_window) {
        if (lcdc & (1 << 3)) {
            base = 0x9C00;
        }
        else {
            base = 0x9800;
        }
    }
    else {
        if (lcdc & (1 << 6)) {
            base = 0x9C00;
        }
        else {
            base = 0x9800;
        }
    }
    uint8_t tile_id = read_byte(base + tv);


    uint16_t start_index;

    if ((lcdc & (1 << 4))) {
        start_index = 0x8000 + (((uint16_t)tile_id) << 4);
    }
    else {
        start_index = 0x9000 + (((int32_t)((int8_t)tile_id)) * 16);
    }

    std::array<uint8_t, 17> data;

    // get attribute
    if (cgb_mode) {
        data[16] = read_vram(base + tv, 1);
    }
    else {
        data[16] = 0;
    }

    if (cgb_mode && (data[16] & (1 << 3))) {
        for (uint32_t i = 0; i < 16; i++) {
            data[i] = read_vram(start_index + i, 1);
        }
    }
    else {
        for (uint32_t i = 0; i < 16; i++) {
            if (start_index + i == 0x8300) {
                good_print = true;
                data[16] = 1;
                // std::cout << "ko ko " << (uint32_t)read_vram(start_index + i, 0) << "\n";
            }
            data[i] = read_vram(start_index + i, 0);
            if (good_print) {
                // std::cout << (uint32_t)data[i] << " " << i << "\n";
            }
        }
    }
    good_print = false;
    return data; 
}

void draw_bg() {
    if (cgb_mode && !(lcdc & 1)) {
        for (uint8_t i = 0; i < WIDTH; i++) {
            frame_buffer[ly][i] = GB_COLOR[0];
        }
        return;
    }

    std::array<uint8_t, 17> tile_data;

    uint8_t scx = read_byte(0xFF43);
    uint8_t scy = read_byte(0xFF42);

    for (uint8_t i = 0; i < WIDTH; i++) {

        uint8_t x = ly + scy;
        uint8_t y = i + scx;
        uint8_t tx = x; uint8_t ty = y;

        x /= 8; y /= 8;
        uint32_t tile_id = (uint32_t)x * 32 + y;

        if (x == 0 && y == 0) {
            should_print = true;
        }

        // if (!(i & LO_3)) {
            tile_data = get_tile_data(tile_id, false);
        // }
        should_print = false;

        if (cgb_mode) {
            uint8_t attribute = tile_data[16];
            uint8_t row = ((attribute & (1 << 5)) ? 7 - (tx & LO_3) : (tx & LO_3));
            uint8_t col = ((attribute & (1 << 6)) ? ty & LO_3 : 7 - (ty & LO_3));
            
            uint8_t palette = attribute & LO_3;
            prio[ly][i] = attribute & (1 << 7);

            uint8_t color_2bit = (((tile_data[2 * row] >> col) & 1) << 1) | ((tile_data[2 * row + 1] >> col) & 1);
            uint8_t color_index = palette * 8 + color_2bit * 2; 
            uint32_t color = read_cram(color_index) | (((uint16_t)read_cram(color_index + 1)) << 8);
            frame_buffer[ly][i] = ((color & LO_5) << 3) | (((color >> 5) & LO_5) << 11) | (((color >> 10) & LO_5) << 19);
        }
        else {
            uint8_t row = tx & LO_3;
            uint8_t col = 7 - (ty & LO_3);

            uint8_t color = (((tile_data[2 * row + 1] >> col) & 1) << 1) | ((tile_data[2 * row] >> col) & 1);
            color = color * (lcdc & 1);
            if (tile_data[16]) {
                // std::cout << "color " << (uint32_t)x << " " << (uint32_t)y << " " << (uint32_t)ly << " " << (uint32_t)i << " " << (uint32_t)row << " " << (uint32_t)col << "\n";
                // std::cout << (uint32_t)color << "\n";
            }
            frame_buffer[ly][i] = GB_COLOR[color];
    
            // frame_buffer[ly][i] = 0xFF00;
        }   
    }
}

void draw_window() {
    // X is row, y is col
    uint8_t wx = read_byte(0xFF4A);
    uint8_t wy = read_byte(0xFF4B);

    std::array<uint8_t, 17> tile_data;

    if (wy >= ly) {
        for (uint8_t i = 0; i < WIDTH; i++) {
            if (wx - 7 >= i) {
                uint8_t x = wy - ly;
                uint8_t y = i - (wx - 7);
                uint8_t tx = x; uint8_t ty = y;

                x /= 8; y /= 8;
                uint32_t tile_id = x * 8 + y;

                // if (!(i & LO_3)) {
                    tile_data = get_tile_data(tile_id, true);
                // }

                if (cgb_mode) {
                    uint8_t attribute = tile_data[16];
                    uint8_t row = ((attribute & (1 << 5)) ? 7 - (tx & LO_3) : (tx & LO_3));
                    uint8_t col = ((attribute & (1 << 6)) ? ty & LO_3 : 7 - (ty & LO_3));
                    
                    uint8_t palette = attribute & LO_3;
                    prio[ly][i] = attribute & (1 << 7);

                    uint8_t color_2bit = (((tile_data[2 * row] >> col) & 1) << 1) | ((tile_data[2 * row + 1] >> col) & 1);
                    uint8_t color_index = palette * 8 + color_2bit * 2; 
                    uint32_t color = read_cram(color_index) | (((uint16_t)read_cram(color_index)) << 8);
                    frame_buffer[ly][i] = ((color & LO_5) << 3) | (((color >> 5) & LO_5) << 11) | (((color >> 10) & LO_5) << 19);
                }
                else {
                    uint8_t row = tx & LO_3;
                    uint8_t col = 7 - (ty & LO_3);

                    uint8_t color = (((tile_data[2 * row + 1] >> col) & 1) << 1) | ((tile_data[2 * row] >> col) & 1);
                    color = color * (lcdc & 1);
                    frame_buffer[ly][i] = GB_COLOR[color];
                }
            }
        }
    }
}

void draw_objs() {
    if (!cgb_mode) {
        std::stable_sort(sprites.begin(), sprites.end(), [&] (const std::array<uint8_t, 4> a, const std::array<uint8_t, 4> b) {
            return a[1] < b[1];
        });
    }

    for (std::array<uint8_t, 4> i : sprites) {
        uint8_t attributes = i[3];
        i[0] -= 16; i[1] -= 8;
        uint8_t height = 8;
        if (lcdc & (1 << 2)) {
            height <<= 1;
        }
        std::cout << "WTF\n";

        if (!cgb_mode) {
            uint8_t relative_x = ly - i[0];

            if (attributes & (1 << 6)) {
                relative_x = (height - 1) - relative_x;
            }


            uint16_t address = 0x8000 + (((uint16_t)i[2]) << 4) + (relative_x << 1);

            for (int8_t j = 7; j >= 0; j--) {
                uint8_t col = j;
                if (attributes & (1 << 5)) {
                    col = 7 - j;
                }

                uint8_t color = ((read_byte(address) >> col) & 1) | (((read_byte(address + 1) >> col) & 1) << 1);
                
                if (color) {
                    if (attributes & (1 << 4)) {
                        frame_buffer[ly][7 - j + i[1]] = GB_COLOR[(read_byte(0xFF49) >> color) & LO_2];
                    }
                    else {
                        frame_buffer[ly][7 - j + i[1]] = GB_COLOR[(read_byte(0xFF48) >> color) & LO_12];
                    }
                }
            }
        }
        else {
            uint8_t relative_x = ly - i[0];

            if (attributes & (1 << 6)) {
                relative_x = (height - 1) - relative_x;
            }

            uint16_t address = 0x8000 + (((uint16_t)i[2]) << 4) + (relative_x << 1);
            uint8_t bank = (attributes >> 3) & 1;

            for (int8_t j = 7; j >= 0; j--) {
                uint8_t col = j;
                if (attributes & (1 << 5)) {
                    col = 7 - j;
                }

                uint8_t color_2bit = ((read_vram(address, bank) >> col) & 1) | (((read_vram(address + 1, bank) >> col) & 1) << 1);
                
                if (color_2bit) {
                    uint8_t palette = attributes & LO_3;
                    uint8_t color_index =  palette * 8 + color_2bit * 2;
                    uint32_t color = read_cram(color_index) | (((uint16_t)read_cram(color_index + 1)) << 8);
                    if (!(lcdc & 1) || (!(attributes & (1 << 7)) && !prio[ly][7 - j + i[1]]) || frame_buffer[ly][7 - j + i[1]] == GB_COLOR[0]) {
                        frame_buffer[ly][7 - j + i[1]] = ((color & LO_5) << 3) | (((color >> 5) & LO_5) << 11) | (((color >> 10) & LO_5) << 19);
                    }
                }
            }
        }
    }
}

void run_draw() {
    if (run_done) {
        return;
    }

    memset(prio, 0, sizeof(prio));

    lcdc = read_byte(0xFF40);

    draw_bg();

    if ((lcdc & (1 << 5)) && (cgb_mode || (lcdc & 1))) {
        draw_window();
    }

    if (lcdc & 2) {
        draw_objs();
    }
    run_done = true;
}


uint32_t run_ppu(uint32_t m_cycles) {
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
            run_draw();
            break;
        }
        case Mode::HBLANK: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)));
            break;
        }
        case Mode::VBLANK: {
            write_byte(0xFF41, (read_byte(0xFF41) & (LO_8 ^ LO_2)) | 1);
            break;
        }
    }
    
    return t_cycles;
}


void reset() {
     for (uint8_t i = 0; i < HEIGHT; i++) {
        for (uint8_t j = 0; j < WIDTH; j++) {
            frame_buffer[i][j] = GB_COLOR[0];
        }
    }

    ly = 0;
    dots = 0;
}
#include "memory.h"
#include "cpu.h"

// pc, sp
uint16_t pc = 0;
uint16_t sp = 0;

// regs
uint8_t a; 
uint8_t flags[4];
uint16_t bc = 0;
uint16_t de = 0;
uint16_t hl = 0;

uint8_t next8() {
    return read_byte(pc++);
}

uint16_t next16() {
    return read_byte(pc++) | (((uint16_t)read_byte(pc++)) << 8);
}

uint8_t get_r8(uint8_t r) {
    switch (r) {
        case 0:
            // b
            return (bc >> 8);
            break;
        case 1:
            // c
            return bc & LO_8;
            break;
        case 2: 
            // d
            return (de >> 8);
            break;
        case 3: 
            // e
            return de & LO_8;
            break;
        case 4:
            // h
            return (hl >> 8);
            break;
        case 5:
            // l
            return hl & LO_8;
            break;
        case 6:
            // [hl]
            return read_byte(hl);
            break;
        case 7:
            // a
            a;
            break;
    }

    return 0;
}

void add_r8(uint8_t r, uint8_t val) {
    switch (r) {
        case 0: {
            // b
            uint8_t b = (bc >> 8);
            uint8_t res = b + val;
            bc = (bc & LO_8) | ((uint16_t)(res) << 8); 
            flags[3] = !res;
            flags[2] = 0;
            flags[1] = (b & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 1: {
            // c
            uint8_t c = bc & LO_8;
            uint8_t res = c + val;
            bc = (bc & HI_8) | res;
            flags[3] = !res;
            flags[2] = 0;
            flags[1] = (c & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 2: {
            // d
            uint8_t d = de >> 8;
            uint8_t res = d + val;
            de = (de & LO_8) | (((uint16_t)(res)) << 8);
            flags[3] = !res; 
            flags[2] = 0;
            flags[1] = (d & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 3: {
            // e
            uint8_t e = de & LO_8;
            uint8_t res = e + val;
            de = (de & HI_8) | res;
            flags[3] = !res;
            flags[2] = 0;
            flags[1] = (e & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 4: {
            // h
            uint8_t h = hl >> 8;
            uint8_t res = h + val;
            hl = (hl & LO_8) | (((uint16_t)(res)) << 8);
            flags[3] = !res; 
            flags[2] = 0;
            flags[1] = (h & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 5: {
            // l
            uint8_t l = hl & LO_8;
            uint8_t res = l + val;
            hl = (hl & HI_8) | res;
            flags[3] = !res;
            flags[2] = 0;
            flags[1] = (l & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 6: {
            // [hl]
            uint8_t hl_v = read_byte(hl);
            uint8_t res = hl_v + val;
            write_byte(hl, res);
            flags[3] = !res;
            flags[2] = 0;
            flags[1] = (hl_v & LO_4) + (val & LO_4) > LO_4;
            break;
        }
        case 7: {
            // a
            uint8_t ta = a;
            a += val;
            flags[3] = !a;
            flags[2] = 0;
            flags[1] = (ta & LO_4) + (val & LO_4) > LO_4;
            break;
        }
    }
}

void set_r8(uint8_t r, uint16_t v) {
    switch (r) {
        case 0:
            bc = (bc & LO_8) | (v << 8);
            break;
        case 1:
            bc = (bc & HI_8) | v;
            break;
        case 2:
            de = (de & LO_8) | (v << 8);
            break;
        case 3:
            de = (de & HI_8) | v;
            break;
        case 4: 
            hl = (hl & LO_8) | (v << 8);
            break;
        case 5: 
            hl = (hl & HI_8) | v;
            break;
        case 6:
            write_byte(hl, v);
            break;
        case 7:
            a = v;
            break;
    }
}

uint16_t get_16mem(uint8_t r) {
    switch (r) {
        case 0: 
            return bc;
            break;
        case 1:
            return de;
            break;
        case 2:
            return hl++;
            break;
        case 3:
            return hl--;
            break;
    }
    
    return 0;
}

uint16_t get_r16(uint8_t r) {
    switch (r) {
        case 0: 
            return bc;
            break;
        case 1:
            return de;
            break;
        case 2: 
            return hl;
            break;
        case 3: 
            return sp;
            break;
    }

    return 0;
}

void set_r16(uint8_t r, uint16_t val) {
    switch (r) {
        case 0: 
            bc = val;
            break;
        case 1:
            de = val;
            break;
        case 2: 
            hl = val;
            break;
        case 3: 
            sp = val;
            break;
    }
}

void add_r16(uint8_t r, uint8_t val) {
    switch (r) {
        case 0:
            bc += val;
            break;
        case 1:
            de += val;
            break;
        case 2: 
            hl += val;
            break;
        case 3:
            sp += val;
            break;
    }
}

void run_cb() {

}

void run00(uint8_t byte) {
    switch (byte & LO_4) {
        case 1: {
            set_r16((byte >> 4) & LO_2, next16());
            break;
        }
        case 2:
            write16(get_16mem((byte >> 4) & LO_2), get_r8(7));
            break;
        case 10: 
            set_r8(7, read_byte(get_16mem((byte >> 4) & LO_2)));
            break;
        case 8: {
            set_r16(next16(), sp);
            break;
        }
        case 3:
            add_r16((byte >> 4) & LO_2, 1);
            break;
        case 11:
            add_r16((byte >> 4) & LO_2, -1);
            break; 
        case 9: {
            uint16_t thl = hl;
            hl += get_r16((byte >> 4) & LO_2);
            flags[2] = 0;
            flags[1] = (thl & LO_12) + (hl & LO_12) > LO_12;
            flags[0] = ((uint32_t)thl + hl) > LO_16;
            break;
        }
        case 4:
            add_r8((byte >> 3) & LO_3, 1);
            break;
        case 12:
            add_r8((byte >> 3) & LO_3, 1);
            break;
        case 5:
            add_r8((byte >> 3) & LO_3, -1);
            break;
        case 13: 
            add_r8((byte >> 3) & LO_3, -1);
            break;
        case 6:
            set_r8((byte >> 3) & LO_3, next8());
        case 14:
            set_r8((byte >> 3) & LO_3, next8());
        case 7: {
            uint8_t hi_4 = byte >> 4;
            switch (hi_4) {
                case 0:
                    flags[0] = a & 128;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | (a >> 7);
                    break;
                case 1:
                    uint8_t tc = flags[3];
                    flags[0] = a & 128;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | tc;
                    break;
                case 2: {
                    // yoinked from: https://blog.ollien.com/posts/gb-daa/ 
                    uint8_t offset = 0;
                    if ((!flags[2] && (a & LO_4) > 9) || flags[1]) {
                        offset = 6;
                    }
                    if ((!flags[2] && a > 153) || flags[0]) {
                        offset |= 96; 
                        flags[0] = 1;
                    }
                    
                    if (!flags[2]) {
                        a += offset;
                    }
                    else {
                        a -= offset;
                    }

                    flags[1] = 0;
                    flags[3] = !a;
                    flags[0] = a > 153; 
                    break; 
                } 
                case 3: 
                    flags[0] = 1;
                    flags[1] = 0; flags[2] = 0;
                    break;
                default:
                    std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
                    exit(1);
                    break;
            }
        }
        case 15:
            break;
    }
}

void run() {
    uint8_t byte = next8();
    switch (byte) {
        case 0:
            return;
            break;
        case CB: 
            run_cb();
            break;
        default:
            // casework on first 2 bits
            switch (byte >> 6) {
                case 0:
                    run00(byte);
                    break;
                case 1:
                    // run01(byte); 
                    break;
                case 2: 
                    // run10(byte);
                    break;
                case 3:
                    // run11(byte);
                    break;
            }
            break;
    }
}
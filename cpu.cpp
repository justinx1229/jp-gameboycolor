#include "memory.h"
#include "cpu.h"
#include "timer.h"
#include "interrupts.h"

// pc, sp
uint16_t pc = 0;
uint16_t sp = 0;

// regs
uint8_t a; 
uint8_t flags[4];
uint16_t bc = 0;
uint16_t de = 0;
uint16_t hl = 0;

bool stopped = false;
uint8_t wake = 0;

void reset_cpu() {
    pc = 0x0100;
    sp = 0xFFFE;

    a = 0x01;
    flags[3] = 1;
    flags[2] = 0;
    flags[1] = 1;
    flags[0] = 1;

    bc = 0x0013;
    de = 0x00D8;
    hl = 0x014D;

    reset_interrupt();
    stopped = false;
    wake = 0;
}

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
            return a;
    }

    return 0;
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

bool check_cond(uint8_t cond) {
    switch (cond) {
        case 0:
            return !flags[3];
        case 1:
            return flags[3];
        case 2:
            return !flags[0];
        case 3:
            return flags[0];
    }

    return false;
}

uint8_t estimate_cycles(uint8_t byte, uint8_t cb_byte) {
    if (byte == 0x00 || byte == 0x10) {
        return 1;
    }

    if (byte == CB) {
        return (cb_byte & LO_3) == 6 ? 4 : 2;
    }

    switch (byte >> 6) {
        case 0:
            switch (byte & LO_4) {
                case 1:
                    return 3;
                case 2:
                case 10:
                case 3:
                case 11:
                case 9:
                    return 2;
                case 4:
                case 5:
                case 12:
                case 13:
                    return ((byte >> 3) & LO_3) == 6 ? 3 : 1;
                case 6:
                case 14:
                    return ((byte >> 3) & LO_3) == 6 ? 3 : 2;
                case 7:
                case 15:
                    return 1;
                case 8:
                    if (byte == 0x08) {
                        return 5;
                    }
                    if (byte == 0x18) {
                        return 3;
                    }
                    return check_cond((byte >> 3) & LO_2) ? 3 : 2;
                case 0:
                    return check_cond((byte >> 3) & LO_2) ? 3 : 2;
            }
            break;
        case 1:
            if (byte == 0x76) {
                return 1;
            }
            return (((byte >> 3) & LO_3) == 6 || (byte & LO_3) == 6) ? 2 : 1;
        case 2:
            return (byte & LO_3) == 6 ? 2 : 1;
        case 3:
            switch (byte) {
                case 0xC6:
                case 0xCE:
                case 0xD6:
                case 0xDE:
                case 0xE6:
                case 0xEE:
                case 0xF6:
                case 0xFE:
                    return 2;
                case 0xC0:
                case 0xC8:
                case 0xD0:
                case 0xD8:
                    return check_cond((byte >> 3) & LO_2) ? 5 : 2;
                case 0xC9:
                case 0xD9:
                    return 4;
                case 0xC2:
                case 0xCA:
                case 0xD2:
                case 0xDA:
                    return check_cond((byte >> 3) & LO_2) ? 4 : 3;
                case 0xC3:
                    return 4;
                case 0xE9:
                    return 1;
                case 0xC4:
                case 0xCC:
                case 0xD4:
                case 0xDC:
                    return check_cond((byte >> 3) & LO_2) ? 6 : 3;
                case 0xCD:
                    return 6;
                case 0xC7:
                case 0xCF:
                case 0xD7:
                case 0xDF:
                case 0xE7:
                case 0xEF:
                case 0xF7:
                case 0xFF:
                    return 4;
                case 0xC1:
                case 0xD1:
                case 0xE1:
                case 0xF1:
                    return 3;
                case 0xC5:
                case 0xD5:
                case 0xE5:
                case 0xF5:
                    return 4;
                case 0xE0:
                case 0xF0:
                    return 3;
                case 0xE2:
                case 0xF2:
                    return 2;
                case 0xEA:
                case 0xFA:
                    return 4;
                case 0xE8:
                    return 4;
                case 0xF8:
                    return 3;
                case 0xF9:
                    return 2;
                case 0xF3:
                case 0xFB:
                    return 1;
            }
            break;
    }

    return 1;
}

void run_cb(uint8_t byte) {
    switch (byte >> 6) {
        case 0: {
            switch (byte >> 3) {
                case 0: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = (v >> 7) & 1;
                    v = (v >> 7) | (v << 1);
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 1: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = v & 1;
                    v = (v >> 1) | ((v & 1) << 7);
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 2: {
                    uint8_t tc = flags[0];
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = (v >> 7) & 1;
                    v = (v << 1) | tc;
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 3: {
                    uint8_t tc = flags[0];
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = v & 1;
                    v = (v >> 1) | (tc << 7);
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 4: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = (v >> 7) & 1;
                    v <<= 1;
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 5: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = v & 1;
                    v = (v >> 1) | (v & 128);
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 6: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    v = ((v & LO_4) << 4) | (v >> 4);
                    flags[3] = !v;
                    flags[0] = 0; flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
                case 7: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = v & 1;
                    v >>= 1;
                    flags[3] = !v;
                    flags[1] = 0; flags[2] = 0;
                    set_r8(r, v);
                    break;
                }
            }
            break;
        }
        case 1: {
            uint8_t bit = (byte >> 3) & LO_3;
            uint8_t r = byte & LO_3;
            flags[3] = !((get_r8(r) >> bit) & 1);
            flags[2] = 0; flags[1] = 1;
            break;
        }
        case 2: {
            uint8_t bit = (byte >> 3) & LO_3;
            uint8_t r = byte & LO_3;
            uint8_t v = get_r8(r);
            v &= LO_8 ^ (1 << bit);
            set_r8(r, v);
            break;
        }
        case 3: {
            uint8_t bit = (byte >> 3) & LO_3;
            uint8_t r = byte & LO_3;
            uint8_t v = get_r8(r);
            v |= (1 << bit);
            set_r8(r, v);
            break;
        }
    }
}

void run00(uint8_t byte) {
    switch (byte & LO_4) {
        case 1: {
            set_r16((byte >> 4) & LO_2, next16());
            break;
        }
        case 2:
            write_byte(get_16mem((byte >> 4) & LO_2), get_r8(7));
            break;
        case 10: 
            set_r8(7, read_byte(get_16mem((byte >> 4) & LO_2)));
            break;
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
        case 4: {
            uint8_t r = (byte >> 3) & LO_3;
            uint8_t v = get_r8(r);
            flags[1] = (1 + (v & LO_4)) > LO_4;
            flags[2] = 0;
            flags[3] = !(++v);
            set_r8(r, v);
            break;
        }
        case 12: {
            uint8_t r = (byte >> 3) & LO_3;
            uint8_t v = get_r8(r);
            flags[1] = (1 + (v & LO_4)) > LO_4;
            flags[2] = 0;
            flags[3] = !(++v);
            set_r8(r, v);
            break;
        }
        case 5: {
            uint8_t r = (byte >> 3) & LO_3;
            uint8_t v = get_r8(r);
            flags[1] = !(v & LO_4);
            flags[2] = 1;
            flags[3] = !(--v);
            set_r8(r, v);
            break;
        }
        case 13: {
            uint8_t r = (byte >> 3) & LO_3;
            uint8_t v = get_r8(r);
            flags[1] = !(v & LO_4);
            flags[2] = 1;
            flags[3] = !(--v);
            set_r8(r, v);
            break;
        }
        case 6:
            set_r8((byte >> 3) & LO_3, next8());
            break;
        case 14:
            set_r8((byte >> 3) & LO_3, next8());
            break;
        case 7: {
            uint8_t hi_4 = byte >> 4;
            switch (hi_4) {
                case 0: {
                    flags[0] = (a >> 7) & 1;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | (a >> 7);
                    break;
                }
                case 1: {
                    uint8_t tc = flags[0];
                    flags[0] = (a >> 7) & 1;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | tc;
                    break;
                }
                case 2: {
                    bool carry = flags[0];

                    if (!flags[2]) {
                        if (carry || a > 0x99) {
                            a += 0x60;
                            carry = true;
                        }
                        if (flags[1] || (a & 0x0F) > 0x09) {
                            a += 0x06;
                        }
                    }
                    else {
                        if (carry) {
                            a -= 0x60;
                        }
                        if (flags[1]) {
                            a -= 0x06;
                        }
                    }

                    flags[0] = carry;
                    flags[1] = 0;
                    flags[3] = !a;
                    break; 
                } 
                case 3: {
                    flags[0] = 1;
                    flags[1] = 0; flags[2] = 0;
                    break;
                }
                default:
                    std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
                    exit(1);
                    break;
            }
            break;
        }
        case 15: {
            uint8_t hi_4 = byte >> 4;
            switch (hi_4) {
                case 0: {
                    flags[0] = a & 1;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & 1) << 7) | (a >> 1);
                    break;
                }
                case 1: {
                    uint8_t tc = flags[0];
                    flags[0] = a & 1;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = (tc << 7) | (a >> 1);
                    break;
                }
                case 2: {
                    a = ~a;
                    flags[2] = 1;
                    flags[1] = 1;
                    break;
                }
                case 3: {
                    flags[2] = 0;
                    flags[1] = 0;
                    flags[0] = !flags[0];
                    break;
                }
                default: 
                    std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
                    exit(1);
                    break;
            }
            break;
        }
        case 8: {
            if (byte == 8) {
                write16(next16(), sp);
            }
            else if (byte == 24) {
                int8_t imm8 = (int8_t)next8();
                pc += imm8;
            }
            else if ((byte >> 5) == 1) {
                int8_t imm8 = (int8_t)next8();
                switch ((byte >> 3) & LO_2) {
                    case 0: {
                        if (!flags[3]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 1: {
                        if (flags[3]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 2: {
                        if (!flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 3: {
                        if (flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                }
            }
            else {
                std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
                exit(1);
            }
            break;
        }
        case 0: {
            if ((byte >> 5) == 1) {
                int8_t imm8 = (int8_t)next8();
                switch ((byte >> 3) & LO_2) {
                    case 0: {
                        if (!flags[3]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 1: {
                        if (flags[3]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 2: {
                        if (!flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 3: {
                        if (flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                }
                break;
            }
            else {
                std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
                exit(1);
            }
            break;
        }
    }
}

void run01(uint8_t byte) {
        if (byte == 0x76) {
            if (!is_ime()) {
                uint8_t ie_ = read_byte(0xFFFF);
                uint8_t if_ = read_byte(0xFF0F);
                if (!(ie_ & if_)) {
                    set_halt(true);
                }
            }
            return;
    }

    uint8_t dest = (byte >> 3) & LO_3;
    uint8_t src = byte & LO_3;

    set_r8(dest, get_r8(src));
}

void alu(uint8_t op, uint8_t val) {
    switch (op) {
        case 0: {
            uint16_t res = (uint16_t)a + val;
            flags[3] = !(res & LO_8);
            flags[2] = 0;
            flags[1] = ((a & LO_4) + (val & LO_4)) > LO_4;
            flags[0] = res > LO_8;
            a = res & LO_8;
            break;
        }
        case 1: {
            uint8_t carry = flags[0];
            uint16_t res = (uint16_t)a + val + carry;
            flags[3] = !(res & LO_8);
            flags[2] = 0;
            flags[1] = ((a & LO_4) + (val & LO_4) + carry) > LO_4;
            flags[0] = res > LO_8;
            a = res & LO_8;
            break;
        }
        case 2: {
            uint8_t old_a = a;
            a = old_a - val;
            flags[3] = !a;
            flags[2] = 1;
            flags[1] = (old_a & LO_4) < (val & LO_4);
            flags[0] = old_a < val;
            break;
        }
        case 3: {
            uint8_t carry = flags[0];
            uint16_t sub = (uint16_t)val + carry;
            uint8_t old_a = a;
            a = old_a - sub;
            flags[3] = !a;
            flags[2] = 1;
            flags[1] = (old_a & LO_4) < ((val & LO_4) + carry);
            flags[0] = old_a < sub;
            break;
        }
        case 4:
            a &= val;
            flags[3] = !a;
            flags[2] = 0;
            flags[1] = 1;
            flags[0] = 0;
            break;
        case 5:
            a ^= val;
            flags[3] = !a;
            flags[2] = 0;
            flags[1] = 0;
            flags[0] = 0;
            break;
        case 6:
            a |= val;
            flags[3] = !a;
            flags[2] = 0;
            flags[1] = 0;
            flags[0] = 0;
            break;
        case 7: {
            uint8_t old_a = a;
            flags[3] = !(uint8_t)(old_a - val);
            flags[2] = 1;
            flags[1] = (old_a & LO_4) < (val & LO_4);
            flags[0] = old_a < val;
            break;
        }
    }
}

void run10(uint8_t byte) {
    uint8_t op = (byte >> 3) & LO_3;
    uint8_t src = byte & LO_3;
    uint8_t val = get_r8(src);

    alu(op, val);
}


void run11(uint8_t byte) {
    switch (byte) {
        // ALU ops
        case 0b11000110:
        case 0b11001110:
        case 0b11010110:
        case 0b11011110:
        case 0b11100110:
        case 0b11101110:
        case 0b11110110:
        case 0b11111110: {
            uint8_t val = next8();
            uint8_t op = (byte >> 3) & LO_3;
            alu(op, val);
            break;
        }
        // conditional return
        case 0b11000000:
        case 0b11001000:
        case 0b11010000:
        case 0b11011000: {
            uint8_t cond = (byte >> 3) & LO_2;
            bool take = false;

            switch (cond) {
                case 0:
                    take = !flags[3];
                    break;
                case 1:
                    take = flags[3];
                    break;
                case 2:
                    take = !flags[0];
                    break;
                case 3:
                    take = flags[0];
                    break;
            }

            if (take) {
                pc = read_byte(sp) | (((uint16_t)read_byte(sp + 1)) << 8);
                sp += 2;
            }
            break;
        }
        // return stuff
        case 0b11001001:
            pc = read_byte(sp) | (((uint16_t)read_byte(sp + 1)) << 8);
            sp += 2;
            break;
        case 0b11011001:
            pc = read_byte(sp) | (((uint16_t)read_byte(sp + 1)) << 8);
            sp += 2;
            set_sime();
            break;
        // cond jump
        case 0b11000010:
        case 0b11001010:
        case 0b11010010:
        case 0b11011010: {
            uint16_t addr = next16();
            uint8_t cond = (byte >> 3) & LO_2;
            bool take = false;

            switch (cond) {
                case 0:
                    take = !flags[3];
                    break;
                case 1:
                    take = flags[3];
                    break;
                case 2:
                    take = !flags[0];
                    break;
                case 3:
                    take = flags[0];
                    break;
            }

            if (take) {
                pc = addr;
            }
            break;
        }
        // jump imm16
        case 0b11000011:
            pc = next16();
            break;
        // jump hl
        case 0b11101001:
            pc = hl;
            break;
        // conditional call
        case 0b11000100:
        case 0b11001100:
        case 0b11010100:
        case 0b11011100: {
            uint16_t addr = next16();
            uint8_t cond = (byte >> 3) & LO_2;
            bool take = false;

            switch (cond) {
                case 0:
                    take = !flags[3];
                    break;
                case 1:
                    take = flags[3];
                    break;
                case 2:
                    take = !flags[0];
                    break;
                case 3:
                    take = flags[0];
                    break;
            }

            if (take) {
                sp--;
                write_byte(sp, pc >> 8);
                sp--;
                write_byte(sp, pc & LO_8);
                pc = addr;
            }
            break;
        }
        // call imm16
        case 0b11001101: {
            uint16_t addr = next16();
            sp--;
            write_byte(sp, pc >> 8);
            sp--;
            write_byte(sp, pc & LO_8);
            pc = addr;
            break;
        }
        // restart
        case 0b11000111:
        case 0b11001111:
        case 0b11010111:
        case 0b11011111:
        case 0b11100111:
        case 0b11101111:
        case 0b11110111:
        case 0b11111111: {
            sp--;
            write_byte(sp, pc >> 8);
            sp--;
            write_byte(sp, pc & LO_8);
            pc = byte & 0b00111000;
            break;
        }
        // pop r16stk
        case 0b11000001:
        case 0b11010001:
        case 0b11100001:
        case 0b11110001: {
            uint16_t val = read_byte(sp) | (((uint16_t)read_byte(sp + 1)) << 8);
            sp += 2;

            switch ((byte >> 4) & LO_2) {
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
                    a = val >> 8;
                    flags[3] = (val >> 7) & 1;
                    flags[2] = (val >> 6) & 1;
                    flags[1] = (val >> 5) & 1;
                    flags[0] = (val >> 4) & 1;
                    break;
            }
            break;
        }
        // push r16stk
        case 0b11000101:
        case 0b11010101:
        case 0b11100101:
        case 0b11110101: {
            uint16_t val = 0;

            switch ((byte >> 4) & LO_2) {
                case 0:
                    val = bc;
                    break;
                case 1:
                    val = de;
                    break;
                case 2:
                    val = hl;
                    break;
                case 3:
                    val = ((uint16_t)a << 8) |
                        ((uint16_t)flags[3] << 7) |
                        ((uint16_t)flags[2] << 6) |
                        ((uint16_t)flags[1] << 5) |
                        ((uint16_t)flags[0] << 4);
                    break;
            }

            sp--;
            write_byte(sp, val >> 8);
            sp--;
            write_byte(sp, val & LO_8);
            break;
        }
        // ldh [imm8], a
        case 0b11100000:
            write_byte(0xFF00 + next8(), a);
            break;
        // ldh [c], a
        case 0b11100010:
            write_byte(0xFF00 + (bc & LO_8), a);
            break;
        // ld [imm16], a
        case 0b11101010:
            write_byte(next16(), a);
            break;
        // ldh a, [imm8]
        case 0b11110000:
            a = read_byte(0xFF00 + next8());
            break;
        // ldh a, [c]
        case 0b11110010:
            a = read_byte(0xFF00 + (bc & LO_8));
            break;
        // ld a, [imm16]
        case 0b11111010:
            a = read_byte(next16());
            break;
        // add sp, imm8
        case 0b11101000: {
            int8_t offset = (int8_t)next8();
            uint16_t old_sp = sp;
            uint16_t unsigned_offset = (uint8_t)offset;

            sp += offset;
            flags[3] = 0;
            flags[2] = 0;
            flags[1] = ((old_sp & LO_4) + (unsigned_offset & LO_4)) > LO_4;
            flags[0] = ((old_sp & LO_8) + (unsigned_offset & LO_8)) > LO_8;
            break;
        }
        // ld hl, sp + imm8
        case 0b11111000: {
            int8_t offset = (int8_t)next8();
            uint16_t unsigned_offset = (uint8_t)offset;

            hl = sp + offset;
            flags[3] = 0;
            flags[2] = 0;
            flags[1] = ((sp & LO_4) + (unsigned_offset & LO_4)) > LO_4;
            flags[0] = ((sp & LO_8) + (unsigned_offset & LO_8)) > LO_8;
            break;
        }
        // ld sp, hl
        case 0b11111001:
            sp = hl;
            break;
        // di
        case 0b11110011:
            disable_interrupts();
            break;
        // ei
        case 0b11111011:
            set_sime();
            break;
        // invalid opcodes
        case 0b11010011:
        case 0b11011011:
        case 0b11011101:
        case 0b11100011:
        case 0b11100100:
        case 0b11101011:
        case 0b11101100:
        case 0b11101101:
        case 0b11110100:
        case 0b11111100:
        case 0b11111101:
            std::cerr << "Invalid instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
            exit(1);
            break;
        default:
            std::cerr << "Unimplemented block 3 instruction " << std::bitset<8>(byte).to_string() << " at pc " << pc << "\n";
            exit(1);
            break;
    }
}

void run() {
    if (stopped) {
        return;
    }

    uint8_t interrupt_cycles = handle_interrupt(pc, sp);
    if (interrupt_cycles) {
        tick_timer(interrupt_cycles);
        sime_to_ime();
        return;
    }

    uint8_t m_cycles = 1;

    if (!is_halt()) {
        if (pc == 0x0100) {
            if (a == 0x0011) {
                cgb_mode = 1;
            }
            else {
                cgb_mode = 0;
            }
        }

        uint8_t byte = next8();
        uint8_t cb_byte = 0;

        if (byte == CB) {
            cb_byte = read_byte(pc);
        }

        m_cycles = estimate_cycles(byte, cb_byte);

        switch (byte) {
            case 0:
                break;
                break;
            case 16:
                // stop, TODO
                if (!wake) {
                    stopped = true;
                }
                break;
            case CB: 
                run_cb(next8());
                break;
            default:
                // casework on first 2 bits
                switch (byte >> 6) {
                    case 0:
                        run00(byte);
                        break;
                    case 1:
                        run01(byte); 
                        break;
                    case 2: 
                        run10(byte);
                        break;
                    case 3:
                        run11(byte);
                        break;
                }
                break;
        }
    }
    else {
        tick_timer(1);
        sime_to_ime();
        return;
    }

    tick_timer(m_cycles);
    sime_to_ime();
}

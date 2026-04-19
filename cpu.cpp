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

bool stopped = false;
uint8_t wake = 0;

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

void run_cb(uint8_t byte) {
    switch (byte >> 6) {
        case 0: {
            switch (byte >> 3) {
                case 0: {
                    uint8_t r = byte & LO_3;
                    uint8_t v = get_r8(r);
                    flags[0] = v >> 7;
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
                    flags[0] = v >> 7;
                    v = (v << 7) | tc;
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
                    flags[0] = v >> 7;
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
                    v = (v >> 1) | (v & 127);
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
                    flags[0] = 0; flags[1] = 0;
                    break;
                }
            }
            break;
        }
        case 1: {
            uint8_t bit = (byte >> 3) & LO_3;
            uint8_t r = byte & LO_3;
            flags[3] = !((get_r8(r) >> bit) & 1);
            flags[2] = 0; flags[1] = 0;
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
            write16(get_16mem((byte >> 4) & LO_2), get_r8(7));
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
            flags[1] = (LO_4 + (v & LO_4)) > LO_4;
            flags[2] = 0;
            flags[3] = !(--v);
            set_r8(r, v);
            break;
        }
        case 13: {
            uint8_t r = (byte >> 3) & LO_3;
            uint8_t v = get_r8(r);
            flags[1] = (LO_4 + (v & LO_4)) > LO_4;
            flags[2] = 0;
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
                    flags[0] = a >> 7;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | (a >> 7);
                    break;
                }
                case 1: {
                    uint8_t tc = flags[0];
                    flags[0] = a >> 7;
                    flags[1] = 0; flags[2] = 0; flags[3] = 0;
                    a = ((a & LO_7) << 1) | tc;
                    break;
                }
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
                set_r16(next16(), sp);
            }
            else if (byte == 24) {
                uint8_t imm8 = next8();
                pc += imm8;
            }
            else if ((byte >> 5) == 1) {
                uint8_t imm8 = next8();
                switch (byte & (1 << 4)) {
                    case 0: {
                        if (!flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 16: {
                        if (flags[3]) {
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
                uint8_t imm8 = next8();
                switch (byte & (1 << 4)) {
                    case 0: {
                        if (flags[0]) {
                            pc += imm8;
                        }
                        break;
                    }
                    case 16: {
                        if (!flags[3]) {
                            pc += imm8;
                        }
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
        // TODO: halt
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
        case 0b11011001:
            pc = read_byte(sp) | (((uint16_t)read_byte(sp + 1)) << 8);
            sp += 2;
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
            break;
        // ei
        case 0b11111011:
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
    
    uint8_t byte = next8();
    switch (byte) {
        case 0:
            return;
            break;
        case 16:
            // stop, TODO
            if (!wake) {
                stopped = true;
            }
            break;
        case CB: 
            run_cb(byte);
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

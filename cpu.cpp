#include "memory.h"
#include "cpu.h"

// pc, sp
uint16_t pc = 0;
uint16_t sp = 0;

// regs
uint16_t af = 0;
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
            return (af >> 8);
            break;
    }

    return 0;
}

void add_r8(uint8_t r, uint8_t val) {
    switch (r) {
        case 0:
            // b
            bc = (bc & LO_8) | (((uint16_t)(((uint8_t)(bc >> 8)) + 1)) << 8); 
            break;
        case 1:
            // c
            bc = (bc & HI_8) | (((uint8_t)(bc & LO_8)) + 1);
            break;
        case 2: 
            // d
            de = (de & LO_8) | (((uint16_t)(((uint8_t)(de >> 8)) + 1)) << 8); 
            break;
        case 3: 
            // e
            de = (de & HI_8) | (((uint8_t)(de & LO_8)) + 1);
            break;
        case 4:
            // h
            hl = (hl & LO_8) | (((uint16_t)(((uint8_t)(hl >> 8)) + 1)) << 8); 
            break;
        case 5:
            // l
            hl = (hl & HI_8) | (((uint8_t)(hl & LO_8)) + 1);
            break;
        case 6:
            // [hl]
            add_byte(hl, val);
            break;
        case 7:
            // a
            af = (af & LO_8) | (((uint16_t)(((uint8_t)(af >> 8)) + 1)) << 8); 
            break;
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
            af = (af & LO_8) | (v << 8);
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
            set_r16((byte >> 4) & LO_2, next16);
            break;
        }
        case 2:
            write16(get_16mem((byte >> 4) & LO_2), get_r8(7));
            break;
        case 10: 
            set_r8(7, read_byte(get_16mem((byte >> 4) & LO_2)));
            break;
        case 8: {
            set_r16(next16, sp);
            break;
        }
        case 3:
            add_r16((byte >> 4) & LO_2, 1);
            break;
        case 11:
            add_r16((byte >> 4) & LO_2, -1);
            break; 
        case 9:
            hl += get_r16((byte >> 4) & LO_2);
            break;
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
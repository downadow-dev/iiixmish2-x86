/*  iiixmish2-x86 -- the iiixmish2 emulator
    See COPYING file for license text.
    Copyright (C) 2025  Menshikov Sviatoslav Sergeevich

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "keyboard.h"
#include "video.h"
#include "types.h"
#include "char.h"
#include "tsc.h"

#include "program.h"

#define MEM_LENGTH    10000000
#define VMEM_LENGTH   2000
#define REG_LENGTH    44
static int32 mem[MEM_LENGTH];
static uint16 vmem[VMEM_LENGTH];
static int32 reg[REG_LENGTH];
static uint64 time_start;

static uint8 real_color(uint8 xm2_color) {
    switch(xm2_color) {
    case 1 : return 15;
    case 2 : return 10;
    case 3 : return 1;
    case 4 : return 2;
    case 5 : return 7;
    case 6 : return 4;
    case 7 : return 14;
    default: return 0;
    }
}

static void get_key(void) {
    uint8 c;
    
    if((c = char_decode()) != 0) {
        reg[1] = c;
    }
}

static void draw(void) {
    video_usecolor(real_color((uint8)vmem[1999]), real_color((uint8)vmem[1998]));
    video_fill(0, 0, 80, 25, ' ');
    
    int i = 0;
    
    for(int y = 0; y < 25; y++) {
        for(int x = 0; x < 63; x++, i++) {
            if(vmem[i] >= 255 && vmem[i] < 265)
                vmem[1999] = vmem[i] - 255;
            else if(vmem[i] != 0) {
                video_usecolor(real_color((uint8)vmem[1999]), real_color((uint8)vmem[1998]));
                video_putchar(x, y, (vmem[i] == 9608) ? 219 : (uint8)vmem[i]);
            }
        }
    }
    
    for(int y = 0; y < 25; y++) {
        for(int x = 63; x < 80 && i < 1890; x++, i++) {
            if(vmem[i] >= 255 && vmem[i] < 265)
                vmem[1999] = vmem[i] - 255;
            else if(vmem[i] != 0) {
                video_usecolor(real_color((uint8)vmem[1999]), real_color((uint8)vmem[1998]));
                video_putchar(x, y, (vmem[i] == 9608) ? 219 : (uint8)vmem[i]);
            }
        }
    }
    
    video_update();
}

void entry(void) {
    correct_tsc();
    
    for(int i = 0; i < MEM_LENGTH; i++) {
        if(i < VMEM_LENGTH - 2) vmem[i] = 0;
        if(i < REG_LENGTH) reg[i] = 0;
        mem[i] = (i < sizeof(program) / sizeof(int32)) ? program[i] : 0;
    }
    
    vmem[1999] = 1;
    vmem[1998] = 0;
    
    video_hidecursor();
    time_start = get_tsc() / clks_per_msec;
    
    for(int pc = 0; pc < MEM_LENGTH; pc++) {
        switch(mem[pc]) {
        case -1: /* NOP */
            reg[0] = reg[0] - 1 + 2 / 2;
            break;
        case -2: /* ADD */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] + reg[mem[pc - 3] & 0x3F];
            break;
        case -3: /* SUB */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] - reg[mem[pc - 3] & 0x3F];
            break;
        case -4: /* MOV */
            reg[mem[pc - 1] & 0x3F] = mem[pc - 2];
            break;
        case -5: /* ILD */
            if(pc > 65535 && (mem[pc - 2] < 65536 || mem[pc - 2] >= 9999872))
                break; /* memory protection */
            reg[mem[pc - 1] & 0x3F] = mem[mem[pc - 2]];
            break;
        case -6: /* VLD */
            reg[mem[pc - 1] & 0x3F] = vmem[mem[pc - 2] & 0x7FF];
            break;
        case -7: /* LD */
            if(mem[pc - 2] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F];
            break;
        case -8: /* ISV */
            if(pc > 65535 && (mem[pc - 1] < 65536 || (mem[pc - 1] >= 9999000 && mem[pc - 1] < 9999100)))
                break; /* memory protection */
            if(mem[pc - 2] == 1)
                get_key();
            mem[mem[pc - 1]] = reg[mem[pc - 2] & 0x3F];
            break;
        case -9: /* VSV */
            if(mem[pc - 2] == 1)
                get_key();
            vmem[mem[pc - 1] & 0x7FF] = reg[mem[pc - 2] & 0x3F];
            break;
        case -10: /* CALL */
            if(pc > 65535 && mem[pc - 2] < 65536)
                break; /* code protection */
            if((mem[pc - 1] & 0x3F) != 0)
                reg[mem[pc - 1] & 0x3F] = pc + 1;
            pc = mem[pc - 2] - 1;
            break;
        case -11: /* IFA */
            if(pc > 65535 && reg[mem[pc - 3] & 0x3F] < 65536)
                break; /* code protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            if(reg[mem[pc - 1] & 0x3F] == reg[mem[pc - 2] & 0x3F])
                pc = reg[mem[pc - 3] & 0x3F] - 1;
            break;
        case -12: /* IFB */
            if(pc > 65535 && reg[mem[pc - 3] & 0x3F] < 65536)
                break; /* code protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            if(reg[mem[pc - 1] & 0x3F] != reg[mem[pc - 2] & 0x3F])
                pc = reg[mem[pc - 3] & 0x3F] - 1;
            break;
        case -13: /* IFC */
            if(pc > 65535 && reg[mem[pc - 3] & 0x3F] < 65536)
                break; /* code protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            if(reg[mem[pc - 1] & 0x3F] > reg[mem[pc - 2] & 0x3F])
                pc = reg[mem[pc - 3] & 0x3F] - 1;
            break;
        case -14: /* IFD */
            if(pc > 65535 && reg[mem[pc - 3] & 0x3F] < 65536)
                break; /* code protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            if(reg[mem[pc - 1] & 0x3F] < reg[mem[pc - 2] & 0x3F])
                pc = reg[mem[pc - 3] & 0x3F] - 1;
            break;
        case -15: /* UPDD */
            draw();
            break;
        case -16: /* OFF */
            if(pc < 65536)
                return;
            else
                pc = 32767;
            break;
        case -17: /* VRST */
            for(int i = 0; i < 1890; i++)
                vmem[i] = 0;
            break;
        case -18: /* JMP */
            if(pc > 65535 && reg[mem[pc - 1] & 0x3F] < 65536)
                break; /* code protection */
            if(mem[pc - 1] == 1)
                get_key();
            pc = reg[mem[pc - 1] & 0x3F] - 1;
            break;
        case -21: /* MUL */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] * reg[mem[pc - 3] & 0x3F];
            break;
        case -22: /* DIV */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] / reg[mem[pc - 3] & 0x3F];
            break;
        case -26: /* INC */
            if(mem[pc - 1] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F]++;
            break;
        case -27: /* DEC */
            if(mem[pc - 1] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F]--;
            break;
        case -28: /* TNP */
            if(mem[pc - 1] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = -reg[mem[pc - 1] & 0x3F];
            break;
        case -29: /* MOD */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] % reg[mem[pc - 3] & 0x3F];
            break;
        case -38: /* LSHIFT */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] << reg[mem[pc - 3] & 0x3F];
            break;
        case -39: /* RSHIFT */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] >> reg[mem[pc - 3] & 0x3F];
            break;
        case -40: /* XOR */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] ^ reg[mem[pc - 3] & 0x3F];
            break;
        case -41: /* OR */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] | reg[mem[pc - 3] & 0x3F];
            break;
        case -42: /* AND */
            if(mem[pc - 2] == 1 || mem[pc - 3] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = reg[mem[pc - 2] & 0x3F] & reg[mem[pc - 3] & 0x3F];
            break;
        case -43: /* TIME */
            reg[mem[pc - 1] & 0x3F] = get_tsc() / clks_per_msec - time_start;
            break;
        case -44: /* TRST */
            time_start = get_tsc() / clks_per_msec;
            break;
        case -45: /* RISV */
            if(pc > 65535 && (reg[mem[pc - 1] & 0x3F] < 65536 || (reg[mem[pc - 1] & 0x3F] >= 9999000 && reg[mem[pc - 1] & 0x3F] < 9999100)))
                break; /* memory protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1)
                get_key();
            mem[reg[mem[pc - 1] & 0x3F]] = reg[mem[pc - 2] & 0x3F];
            break;
        case -46: /* RVSV */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1)
                get_key();
            vmem[reg[mem[pc - 1] & 0x3F]] = reg[mem[pc - 2] & 0x3F];
            break;
        case -47: /* RILD */
            if(pc > 65535 && (reg[mem[pc - 2] & 0x3F] < 65536 || reg[mem[pc - 2] & 0x3F] >= 9999872))
                break; /* memory protection */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = mem[reg[mem[pc - 2] & 0x3F]];
            break;
        case -48: /* RVLD */
            if(mem[pc - 1] == 1 || mem[pc - 2] == 1)
                get_key();
            reg[mem[pc - 1] & 0x3F] = vmem[reg[mem[pc - 2] & 0x3F]];
            break;
        }
    }
}


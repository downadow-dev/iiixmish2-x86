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
#define REG_LENGTH    28
static int32 mem[MEM_LENGTH];
static int32 vmem[VMEM_LENGTH];
static int32 reg[REG_LENGTH];
static uint64 time_start;
static int32 port = -1;

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
        vmem[1900] = c;
    }
}

static void display(void) {
    if(port != 0) return;
    if(vmem[1901] > 0) {
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
        
        vmem[1901] = 0;
    }
    if(vmem[1902] > 0) {
        for(int i = 0; i <= 1890; i++)
            vmem[i] = 0;
        vmem[1902] = 0;
    }
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
    time_start = get_tsc() / clks_per_msec / 10;
    
    for(int pc = 1; pc < MEM_LENGTH; pc++) {
        if(pc == 0 || mem[pc] >= 0) continue;
        int a25 = mem[pc - 1] >> 6;
        int a6 = mem[pc - 1] & 0x3f;
        int b25 = (-mem[pc]) >> 6;
        int instr = (-mem[pc]) & 0x3f;
        
        switch(instr) {
        case 1: /* NOP */
            a6 <<= 1;
            break;
        case 2: /* ADD */
            reg[b25] = reg[a25] + reg[a6];
            break;
        case 3: /* SUB */
            reg[b25] = reg[a25] - reg[a6];
            break;
        case 4: /* MOV */
            reg[b25] = a25;
            break;
        case 5: /* ILD */
            if(pc > (mem[0]-1) && b25 < mem[0])
                continue; /* memory protection */
            reg[a6] = mem[b25];
            break;
        case 6: /* OPEN */
            if(pc < mem[0]) port = b25;
            break;
        case 7: /* ISV */
            if(pc > (mem[0]-1) && b25 < mem[0])
                continue; /* memory protection */
            mem[b25] = reg[a6];
            break;
        case 8: /* CALL */
            if(pc > (mem[0]-1) && b25 < mem[0])
                continue; /* code protection */
            reg[a6] = pc + 1;
            pc = b25 - 1;
            break;
        case 9: /* IFA */
            if(pc > (mem[0]-1) && reg[a25] < mem[0])
                continue; /* code protection */
            if(reg[b25] == reg[a6]) pc = reg[a25] - 1;
            break;
        case 10: /* IFB */
            if(pc > (mem[0]-1) && reg[a25] < mem[0])
                continue; /* code protection */
            if(reg[b25] != reg[a6]) pc = reg[a25] - 1;
            break;
        case 11: /* IFC */
            if(pc > (mem[0]-1) && reg[a25] < mem[0])
                continue; /* code protection */
            if(reg[b25] > reg[a6]) pc = reg[a25] - 1;
            break;
        case 12: /* IFD */
            if(pc > (mem[0]-1) && reg[a25] < mem[0])
                continue; /* code protection */
            if(reg[b25] < reg[a6]) pc = reg[a25] - 1;
            break;
        case 13: /* OFF */
            pc = (pc < mem[0] ? MEM_LENGTH : 511);
            break;
        case 14: /* JMP */
            if(pc > (mem[0]-1) && reg[b25] < mem[0])
                continue; /* code protection */
            pc = reg[b25] - 1;
            break;
        case 15: /* MUL */
            reg[b25] = reg[a25] * reg[a6];
            break;
        case 16: /* DIV */
            reg[b25] = reg[a25] / reg[a6];
            break;
        case 17: /* INC */
            reg[b25]++;
            break;
        case 18: /* DEC */
            reg[b25]--;
            break;
        case 19: /* TNP */
            reg[b25] = -reg[a25];
            break;
        case 20: /* MOD */
            reg[b25] = reg[a25] % reg[a6];
            break;
        case 21: /* LSHIFT */
            reg[b25] = reg[a25] << reg[a6];
            break;
        case 22: /* RSHIFT */
            reg[b25] = (int32)((uint32)reg[a25] >> (uint32)reg[a6]);
            break;
        case 23: /* XOR */
            reg[b25] = reg[a25] ^ reg[a6];
            break;
        case 24: /* OR */
            reg[b25] = reg[a25] | reg[a6];
            break;
        case 25: /* AND */
            reg[b25] = reg[a25] & reg[a6];
            break;
        case 26: /* TIME */
            reg[b25] = get_tsc() / clks_per_msec / 10 - time_start;
            break;
        case 27: /* TRST */
            time_start = get_tsc() / clks_per_msec / 10;
            break;
        case 28: /* RISV */
            if(pc > (mem[0]-1) && reg[b25] < mem[0])
                continue; /* memory protection */
            mem[reg[b25]] = reg[a6];
            break;
        case 29: /* RILD */
            if(pc > (mem[0]-1) && reg[b25] < mem[0])
                continue; /* memory protection */
            reg[a6] = mem[reg[b25]];
            break;
        case 30: /* RVSV */
            vmem[reg[b25]] = reg[a6];
            if(port == 0) display();
            break;
        case 31: /* RVLD */
            if(port == 0 && reg[b25] == 1900) get_key();
            reg[a6] = vmem[reg[b25]];
            break;
        case 32: /* VSV */
            vmem[b25] = reg[a6];
            if(port == 0) display();
            break;
        case 33: /* VLD */
            if(port == 0 && b25 == 1900) get_key();
            reg[a6] = vmem[b25];
            break;
        }
    }
}


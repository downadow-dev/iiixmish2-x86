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
#define REG_LENGTH    24
static int32 mem[MEM_LENGTH];
static int32 vmem[VMEM_LENGTH];
static int32 reg[REG_LENGTH];
static uint64 time_start, st;
static int32 port = -1, offpc = 0;
static int32 unpriv, interrupt, syscall;
static uint8 hi, savedhi;

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
        mem[i] = (i < sizeof(program) / sizeof(int32)) ?
            (((uint32)program[i * sizeof(int32)] << 24) | ((uint32)program[i * sizeof(int32) + 1] << 16) |
             ((uint32)program[i * sizeof(int32) + 2] << 8)  | (uint32)program[i * sizeof(int32) + 3])
            : 0;
    }
    unpriv = mem[0];
    interrupt = (mem[1] < unpriv ? mem[1] : unpriv);
    syscall = (mem[2] < unpriv ? mem[2] : unpriv);
    
    vmem[1999] = 1;
    vmem[1998] = 0;
    
    video_hidecursor();
    time_start = get_tsc() / clks_per_msec / 10;
    st = get_tsc() / clks_per_msec;
    
    int ticks = 0;
    uint64 res;
    for(int pc = 1; pc < MEM_LENGTH; pc++) {
        if(pc >= unpriv) ticks++;
        if(pc >= unpriv && ticks >= 100000) {
            offpc = pc;
            savedhi = hi;
            pc = interrupt;
            ticks = 0;
        }
        if(pc < 3 || mem[pc] >= 0) continue;
        int32 a = mem[pc - 1] & 0x7fffffff;
        int32 b = (-mem[pc]) >> 11;
        int32 c = ((-mem[pc]) >> 6) & 0x1f;
        int32 d = ((-mem[pc]) >> 5) & 0x1;
        int32 instr = (-mem[pc]) & 0x1f;
        
        switch(instr) {
        case 1: /* VSV */
            vmem[a] = reg[b];
            if(port == 0) display();
            break;
        case 2: /* ADD */
            res = (d == 0 ? ((int64)reg[a] + (int64)reg[b]) : ((uint64)(uint32)reg[a] + (uint64)(uint32)reg[b]));
            reg[c] = (int32)(res & 0xffffffffU);
            hi = ((d == 0 ? (int64)reg[c] : (uint64)(uint32)reg[c]) != res ? 1 : 0);
            break;
        case 3: /* SUB */
            res = (d == 0 ? ((int64)reg[a] - (int64)reg[b]) : ((uint64)(uint32)reg[a] - (uint64)(uint32)reg[b]));
            reg[c] = (int32)(res & 0xffffffffU);
            hi = ((d == 0 ? (int64)reg[c] : (uint64)(uint32)reg[c]) != res ? 1 : 0);
            break;
        case 4: /* MOV */
            reg[c] = a;
            break;
        case 5: /* ILD */
            if(d == 0) {
                if(pc >= unpriv && a < unpriv)
                    continue; /* memory protection */
                reg[b] = mem[a];
                break;
            } else {
                if(pc >= unpriv && a/4 < unpriv)
                    continue; /* memory protection */
                reg[b] = ((uint32)mem[a/4] >> (24 - a % 4 * 8)) & 0xff;
                break;
            }
        case 6: /* OPEN */
            if(pc < unpriv) port = b;
            break;
        case 7: /* ISV */
            if(d == 0) {
                if(pc >= unpriv && a < unpriv)
                    continue; /* memory protection */
                mem[a] = reg[b];
                break;
            } else {
                if(pc >= unpriv && a/4 < unpriv)
                    continue; /* memory protection */
                mem[a/4] = (mem[a/4] & ~(0xff << (24 - a % 4 * 8))) | ((reg[b] & 0xff) << (24 - a % 4 * 8));
                break;
            }
        case 8: /* CALL */
            if(pc >= unpriv && a < unpriv)
                continue; /* code protection */
            reg[b] = pc + 1;
            pc = a - 1;
            break;
        case 9: /* RVLD */
            if(port == 0 && reg[a] == 1900) get_key();
            reg[b] = vmem[reg[a]];
            break;
        case 10: /* IFA */
            hi = (reg[c] == reg[b] ? 1 : 0);
            break;
        case 11: /* IFB */
            hi = ((d == 0 ? (reg[c] < reg[b]) : ((uint32)reg[c] < (uint32)reg[b])) ? 1 : 0);
            break;
        case 12: /* THEN */
            if(pc >= unpriv && reg[c] < unpriv)
                continue; /* memory protection */
            if(hi) pc = reg[c] - 1;
            break;
        case 13: /* OFF */
            pc = (pc < unpriv ? offpc : syscall) - 1;
            hi = savedhi;
            break;
        case 14: /* JMP */
            if(pc >= unpriv && reg[c] < unpriv)
                continue; /* code protection */
            pc = reg[c] - 1;
            break;
        case 15: /* MUL */
            res = (d == 0 ? ((int64)reg[a] * (int64)reg[b]) : ((uint64)(uint32)reg[a] * (uint64)(uint32)reg[b]));
            reg[c] = (int32)(res & 0xffffffffU);
            hi = ((d == 0 ? (int64)reg[c] : (uint64)(uint32)reg[c]) != res ? 1 : 0);
            break;
        case 16: /* DIV */
            if((hi = ((reg[b] == 0 || (d == 0 && reg[a] == (int32)(1U<<31) && reg[b] == -1)) ? 1 : 0)) == 0)
                reg[c] = (d == 0 ? (reg[a] / reg[b]) : ((uint32)reg[a] / (uint32)reg[b]));
            break;
        case 17: /* INC */
            reg[c]++;
            break;
        case 18: /* DEC */
            reg[c]--;
            break;
        case 19: /* TNP */
            if((hi = (reg[b] == (int32)(1U<<31) ? 1 : 0)) == 0)
                reg[c] = -reg[b];
            break;
        case 20: /* REM */
            if((hi = ((reg[b] == 0 || (d == 0 && reg[a] == (int32)(1U<<31) && reg[b] == -1)) ? 1 : 0)) == 0)
                reg[c] = (d == 0 ? (reg[a] % reg[b]) : ((uint32)reg[a] % (uint32)reg[b]));
            break;
        case 21: /* LSHIFT */
            res = ((uint64)reg[a] << reg[b]);
            reg[c] = (int32)(res & 0xffffffffU);
            hi = ((uint32)reg[c] != res ? 1 : 0);
            break;
        case 22: /* RSHIFT */
            reg[c] = ((uint32)reg[a] >> reg[b]);
            break;
        case 23: /* XOR */
            reg[c] = (reg[a] ^ reg[b]);
            break;
        case 24: /* OR */
            reg[c] = (reg[a] | reg[b]);
            break;
        case 25: /* AND */
            reg[c] = (reg[a] & reg[b]);
            break;
        case 26: /* TIME */
            reg[c] = get_tsc() / clks_per_msec / 10 - time_start;
            break;
        case 27: /* VLD */
            if(port == 0 && a == 1900) get_key();
            reg[b] = vmem[a];
            break;
        case 28: /* RISV */
            if(d == 0) {
                if(pc >= unpriv && reg[a] < unpriv)
                    continue; /* memory protection */
                mem[reg[a]] = reg[b];
                reg[a] += c;
                break;
            } else {
                if(pc >= unpriv && reg[a]/4 < unpriv)
                    continue; /* memory protection */
                mem[reg[a]/4] = (mem[reg[a]/4] & ~(0xff << (24 - reg[a] % 4 * 8))) | ((reg[b] & 0xff) << (24 - reg[a] % 4 * 8));
                reg[a] += c;
                break;
            }
        case 29: /* RILD */
            if(d == 0) {
                if(pc >= unpriv && (reg[a] -= c) < unpriv)
                    continue; /* memory protection */
                reg[b] = mem[reg[a]];
                break;
            } else {
                if(pc >= unpriv && (reg[a] -= c)/4 < unpriv)
                    continue; /* memory protection */
                reg[b] = ((uint32)mem[reg[a] / 4] >> (24 - reg[a] % 4 * 8)) & 0xff;
                break;
            }
        case 30: /* RVSV */
            vmem[reg[a]] = reg[b];
            if(port == 0) display();
            break;
        default: break;
        }
    }
}


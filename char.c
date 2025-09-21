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

#include "char.h"
#include "keyboard.h"

static bool shift = false;
static bool ctrl  = false;
static uint8 key;
static bool pressed;
static uint8 c;

uint8 char_decode(void) {
    key_polling();
    key_decode(&key, &pressed);
    
    if(key == 0x3A && pressed) {
        shift = !shift;
        return 0;
    } else if(key == 0x2A || key == 0x36) {
        shift = pressed;
        return 0;
    } else if(key == 0x1D) {
        ctrl = pressed;
        shift = pressed;
        return 0;
    }
    
    c = 0;
    
    if(pressed) {
        if(key == 0x10 && !shift)
            c = 'q';
        else if(key == 0x11 && !shift)
            c = 'w';
        else if(key == 0x12 && !shift)
            c = 'e';
        else if(key == 0x13 && !shift)
            c = 'r';
        else if(key == 0x14 && !shift)
            c = 't';
        else if(key == 0x15 && !shift)
            c = 'y';
        else if(key == 0x16 && !shift)
            c = 'u';
        else if(key == 0x17 && !shift)
            c = 'i';
        else if(key == 0x18 && !shift)
            c = 'o';
        else if(key == 0x19 && !shift)
            c = 'p';
        else if(key == 0x1A && !shift)
            c = '[';
        else if(key == 0x1B && !shift)
            c = ']';
        else if(key == 0x2B && !shift)
            c = '\\';
        else if(key == 0x1E && !shift)
            c = 'a';
        else if(key == 0x1F && !shift)
            c = 's';
        else if(key == 0x20 && !shift)
            c = 'd';
        else if(key == 0x21 && !shift)
            c = 'f';
        else if(key == 0x22 && !shift)
            c = 'g';
        else if(key == 0x23 && !shift)
            c = 'h';
        else if(key == 0x24 && !shift)
            c = 'j';
        else if(key == 0x25 && !shift)
            c = 'k';
        else if(key == 0x26 && !shift)
            c = 'l';
        else if(key == 0x27 && !shift)
            c = ';';
        else if(key == 0x28 && !shift)
            c = '\'';
        else if(key == 0x2C && !shift)
            c = 'z';
        else if(key == 0x2D && !shift)
            c = 'x';
        else if(key == 0x2E && !shift)
            c = 'c';
        else if(key == 0x2F && !shift)
            c = 'v';
        else if(key == 0x30 && !shift)
            c = 'b';
        else if(key == 0x31 && !shift)
            c = 'n';
        else if(key == 0x32 && !shift)
            c = 'm';
        else if(key == 0x33 && !shift)
            c = ',';
        else if(key == 0x34 && !shift)
            c = '.';
        else if(key == 0x35 && !shift)
            c = '/';
        else if(key == 0x29 && !shift)
            c = '`';
        else if(key == 0x02 && !shift)
            c = '1';
        else if(key == 0x03 && !shift)
            c = '2';
        else if(key == 0x04 && !shift)
            c = '3';
        else if(key == 0x05 && !shift)
            c = '4';
        else if(key == 0x06 && !shift)
            c = '5';
        else if(key == 0x07 && !shift)
            c = '6';
        else if(key == 0x08 && !shift)
            c = '7';
        else if(key == 0x09 && !shift)
            c = '8';
        else if(key == 0x0A && !shift)
            c = '9';
        else if(key == 0x0B && !shift)
            c = '0';
        else if(key == 0x0C && !shift)
            c = '-';
        else if(key == 0x0D && !shift)
            c = '=';
        
        else if(key == 0x10)
            c = 'Q';
        else if(key == 0x11)
            c = 'W';
        else if(key == 0x12)
            c = 'E';
        else if(key == 0x13)
            c = 'R';
        else if(key == 0x14)
            c = 'T';
        else if(key == 0x15)
            c = 'Y';
        else if(key == 0x16)
            c = 'U';
        else if(key == 0x17)
            c = 'I';
        else if(key == 0x18)
            c = 'O';
        else if(key == 0x19)
            c = 'P';
        else if(key == 0x1A)
            c = '{';
        else if(key == 0x1B)
            c = '}';
        else if(key == 0x2B)
            c = '|';
        else if(key == 0x1E)
            c = 'A';
        else if(key == 0x1F)
            c = 'S';
        else if(key == 0x20)
            c = 'D';
        else if(key == 0x21)
            c = 'F';
        else if(key == 0x22)
            c = 'G';
        else if(key == 0x23)
            c = 'H';
        else if(key == 0x24)
            c = 'J';
        else if(key == 0x25)
            c = 'K';
        else if(key == 0x26)
            c = 'L';
        else if(key == 0x27)
            c = ':';
        else if(key == 0x28)
            c = '"';
        else if(key == 0x2C)
            c = 'Z';
        else if(key == 0x2D)
            c = 'X';
        else if(key == 0x2E)
            c = 'C';
        else if(key == 0x2F)
            c = 'V';
        else if(key == 0x30)
            c = 'B';
        else if(key == 0x31)
            c = 'N';
        else if(key == 0x32)
            c = 'M';
        else if(key == 0x33)
            c = '<';
        else if(key == 0x34)
            c = '>';
        else if(key == 0x35)
            c = '?';
        else if(key == 0x29)
            c = '~';
        else if(key == 0x02)
            c = '!';
        else if(key == 0x03)
            c = '@';
        else if(key == 0x04)
            c = '#';
        else if(key == 0x05)
            c = '$';
        else if(key == 0x06)
            c = '%';
        else if(key == 0x07)
            c = '^';
        else if(key == 0x08)
            c = '&';
        else if(key == 0x09)
            c = '*';
        else if(key == 0x0A)
            c = '(';
        else if(key == 0x0B)
            c = ')';
        else if(key == 0x0C)
            c = '_';
        else if(key == 0x0D)
            c = '+';
        
        else if(key == 0x01)
            c = 033;
        else if(key == 0x3B)
            c = 0xFF;
        else if(key == 0x0F)
            c = '\t';
        else if(key == 0x39)
            c = ' ';
        else if(key == 0x48)
            c = 'W';
        else if(key == 0x50)
            c = 'S';
        else if(key == 0x4B)
            c = 'A';
        else if(key == 0x4D)
            c = 'D';
        else if(key == 0x1C)
            c = '\n';
        else if(key == 0x0E || key == 0x53)
            c = '\b';
        
        if(ctrl) {
            c -= 64;
        }
    }
    
    return c;
}


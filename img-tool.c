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
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    char buf[12];
    
    printf("static int32 program[] = {\n");
    
    while(fgets(buf, sizeof(buf), stdin)) {
        printf("\t%d,\n", atoi(buf));
    }
    
    printf("};\n");
}


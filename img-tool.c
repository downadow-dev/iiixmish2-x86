/* author: downadow */

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


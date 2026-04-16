#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/go.h"

Value readFile() {
    char* fileName = receive(me()).asString;
    FILE* f = fopen(fileName,"r");
    if (f == 0) {
        perror(fileName);
        exit(1);
    }
    while (1) {
        int c = fgetc(f);
        if (c == EOF) break;
        send(me(), asChar(c));
    }
    fclose(f);
    return asChar(0);
}

int main() {
    Channel* child = go(readFile);
    send(child,asString("t7.ok"));

    while(1) {
        char c = receive(child).asChar;
        if (c == 0) return 0;
        printf("%c",c);
    }

    printf("\n\nDO'T PANIC!\n");
    return 0;
}

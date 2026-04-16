#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "src/go.h"

void go_main() {
    auto ch = go([fileName="t7.ok"] {
        FILE* f = fopen(fileName,"r");
        if (f == 0) {
            perror(fileName);
            exit(1);
        }
        while (1) {
            int c = fgetc(f);
            if (c == EOF) break;
            send(me(), char(c));
        }
        fclose(f);
	return char(0);
    });

    while(1) {
        auto c = receive<char>(ch);
        if (c == 0) return;
        printf("%c",c);
    }

    printf("\n\nDO'T PANIC!\n");
}

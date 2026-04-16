#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

void go_main() {
    printf("[parent] this should be the first line\n");
    auto child = go([] {
        printf("[child] starting\n");
        auto x = receive<int>(me());
        printf("[child] x = %d\n", x);
        return x+1;
    });
    send(child,100);
    printf("[parent] %d\n",receive<int>(child));
    printf("[parent] this should be the last line\n");
}

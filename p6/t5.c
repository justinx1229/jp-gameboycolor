#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

int counter = 0;

void h() {
    send(me(),asInt(counter));
    if (counter < 10) {
        counter ++;
        again();
    }
}

void g() {
    h();
}

Value f() {
    g();
    return asInt(666);
}

int main() {
    Channel* child = go(f);

    while (1) {
        int x = receive(child).asInt;
        if (x == 666) return 0;
        printf("%d\n",x);
    }

    printf("never\n");

    return 0;
}

#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

Value func() {
    Channel* ch = channel();
    for (uint64_t i=0; i<10; i++) {
        send(me(),asLongLong(i));
    }
    receive(ch);
    return asLong(0);
}

int main() {
    Channel* ch = go(func);


    while (1) {
        long long v = receive(ch).asLongLong;
        printf("received %lld\n",v);
    }

    printf("the impossible has happened\n");

    return 0;
}

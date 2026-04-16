#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

Value echo() {
    long x = receive(me()).asLong;
    send(me(),asLong(x+1));
    return asLong(0);
}

int main() {
    Channel* ch = go(echo);
    send(ch,asLong(100));
    printf("%ld\n",receive(ch).asLong);
    printf("this should be the last line\n");
    send(ch,asLong(0));
    printf("should never see this\n");

    return 0;
}

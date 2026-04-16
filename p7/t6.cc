#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

void factorials() {
    int64_t f = 1;
    int64_t n = 0;

    while (1) {
        send(me(),f);
        n++;
        f = f * n;
    }
}

void sumOfFactorials() {
    auto facts = go([] {
        factorials();
	return 0;
    });

    int64_t sum = 0;
    while (1) {
        send(me(), sum += receive<int64_t>(facts));
    }
}

void go_main() {
    auto ch1 = go([] {
        factorials();
	return 0;
    });

    auto ch2 = go([] {
        sumOfFactorials();
        return 0;
    });

    for (long i = 0; i<20; i++) {
        auto f = receive<int64_t>(ch1);
        auto s = receive<int64_t>(ch2);
        printf("i=%" PRIi64 ", f=%" PRIi64 ", s=%" PRIi64 "\n",i,f,s);
    }
}

#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

constexpr int N = 10;

void go_main() {
    auto child = go([] {
        for (int64_t i=0; i<N; i++) {
            send(me(),i);
        }
        receive(chan());
	    printf("the universe is upside down\n");
	    return 0;
   });

    for (int i=0; i<N; i++) {
        auto v = receive<int64_t>(child);
        printf("received %" PRIi64 "\n",v);
    }

    printf("all good\n");
}

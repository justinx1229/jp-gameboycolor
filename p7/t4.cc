#include <stdio.h>
#include <inttypes.h>

#include "src/go.h"

static Chan make_child(uint64_t depth) {
    auto child = go([depth, parent=me()] {
        Chan child = nullptr;
        if (depth > 0) {
            child = make_child(depth-1);
        }
        while (1) {
            auto v = receive<uint64_t>(me());
            if (child != 0) {
                send(child,v+1);
                v = receive<uint64_t>(me());
            }
            send(parent,v+1);
        }
   
        printf("the impossible has happened\n");
	    return 0;
    });
    return child;
}
    
void go_main() {
    Chan child = make_child(100000);

    for (uint64_t i=0; i<12; i++) {
        send(child,i);
        auto v = receive<uint64_t>(me());
        printf("%" PRIu64 "\n",v);
    }

    printf("done\n");

}

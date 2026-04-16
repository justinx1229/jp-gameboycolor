#include <thread>
#include "go.h"

extern void go_main();

constexpr static int N = 4;

int main() {
    go([] {
        go_main();
        exit(0);
        return 0;
    });
    for (int i=1; i<N; i++) {
        std::thread t([] {
            stop();
        });
        t.detach();
    }
    stop();
}


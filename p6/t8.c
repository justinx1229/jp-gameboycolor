#include <stdio.h>
#include <sys/resource.h>
#include "src/go.h"

// Overview: This is a simple stress test that ensures you're freeing your coroutines once they
// terminate.

// Explanation: Without factoring in any malloc calls due to initializing routines, your program
// should use roughly ~1000KB of memory (DEFAULT_USAGE). Just in case your program is extra big
// for some reason, this is multiplied by a factor (LENIENCY) to find the total baseline amount
// of memory your program can use. Next, a bunch of malloc calls are made (M_CALLS) to try and use
// up all of your memory, which shouldn't happen if you free finished coroutines. Overhead of 1kb
// is allotted for each malloc call (M_OVERHEAD), but if you're not freeing then this won't be
// enough to pass the test.

// Implementation Hints: You can't free a finished coroutine in its own call stack, but you can add
// it to some data structure that tracks freed coroutines. Once you're in a different call stack (i.e
// after a context switch) you can free all of these routines. Hint: Don't free the associated Channel

// these are all in kb
#define DEFAULT_USAGE 1000
#define LENIENCY 10
#define M_CALLS 20000
#define M_OVERHEAD 1

Value func()
{
    return asLong(0);
}

int main()
{
    // logic to set the memory limit
    struct rlimit lim;
    getrlimit(RLIMIT_AS, &lim);
    lim.rlim_cur = ((DEFAULT_USAGE * LENIENCY) + (M_CALLS * M_OVERHEAD)) * 1000;
    setrlimit(RLIMIT_AS, &lim);

    // malloc calls
    for (int i = 0; i < M_CALLS; i++)
    {
        Channel *ch = go(func);
        long long v = receive(ch).asLongLong;
        while (v)
            ;
    }
    // success
    printf("Congrats! Didn't run out of memory\n");
    return 0;
}
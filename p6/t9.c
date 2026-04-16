// subset of 035 tests 
// tests stack alignment

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "src/go.h"

Value stack_aligment_test() {
    float val = 222.42;
    printf("%.2f\n", val);
    return asLong(0);
}

int main() {
    Channel * ch = go(stack_aligment_test);
    receive(ch);
    printf("alignment test passed\n");
}
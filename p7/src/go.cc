#include "go.h"
#include <stdlib.h>
#include <stdio.h>
#include <atomic>
#include <cassert>
#include <thread>
#include <iostream>

[[noreturn]]
void missing(const char* file, const int line) {
    fprintf(stderr, "Missing code at %s:%d\n", file, line);
    exit(-1);
}

#define MISSING() missing(__FILE__, __LINE__)

Chan go(std::function<Value()> func) {
    MISSING();
}

Chan me() {
    MISSING();
}

Chan chan() {
    MISSING();
}

Value receive(Chan ch) {
    MISSING();
}

void send(Chan ch, Value v) {
    MISSING();
}

[[noreturn]]
void stop() {
    MISSING();
}

void yield() {
    MISSING();
}



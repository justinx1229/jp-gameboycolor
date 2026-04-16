#include "go.h"
#include <stdlib.h>
#include <stdio.h>

static void missing(const char* file, int line) {
    printf("*** missing code at %s:%d\n",file,line);
    exit(-1);
}

#define MISSING() missing(__FILE__,__LINE__)

struct Routine;
typedef struct Routine Routine;

#define STACK_ENTRIES (8192 / sizeof(uint64_t))

typedef struct Queue {
    struct Routine* head;
    struct Routine* tail;
} Queue;

struct Channel {
};

struct Routine {
    uint64_t saved_rsp;
    // Each c-routine needs its private stack
    uint64_t stack[STACK_ENTRIES];
    Routine* next;
    Channel ch;
};

/////////

static void addQ(Queue* q, Routine* r) {
    r->next = 0;
    if (q->tail != 0) {
        q->tail->next = r;
    }
    q->tail = r;

    if (q->head == 0) {
        q->head = r;
    }
}

static Routine* removeQ(Queue* q) {
    Routine* r = q->head;
    if (r != 0) {
        q->head = r->next;
        if (q->tail == r) {
            q->tail = 0;
        }
    }
    return r;
}

///////////////////////////////////////////////////

static Routine *current_ = 0;
static Queue ready = { 0, 0};

void delete_this_function_when_you_are_done() {
    addQ(&ready,0);
    (void) removeQ(&ready);
}

Routine** current() {
    if (current_ == 0) {
        current_ = (Routine*) calloc(sizeof(Routine),1);
    }
    return &current_;
}

/* OSX prepends _ in front of external symbols */
Routine** _current() {
    return current();
}

Channel* go(Func func) {
    MISSING();
    return 0;
}

Channel* me() {
    return &(*current())->ch;
}

void again() {
    MISSING();
}

Channel* channel() {
    MISSING();
    return 0;
}

Value receive(Channel* ch) {
    MISSING();
    return asLong(666);
}

void send(Channel* ch, Value v) {
    MISSING();
}

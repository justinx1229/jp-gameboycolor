#pragma once

#include <stdint.h>
#include <functional>
#include <variant>
#include <memory>


// Inteneded to be small and passed by value
using Chan = void*; // change for your implementation

// C++ variants are "Discriminated Unions"
using Value = std::variant<char, int, int64_t, uint64_t, void*, Chan*, const char*>;

// schedule a new coroutine that runs func when it's dispatched
// returns the associated channel
// the coroutine can discover its associated channel by calling me()
// the associated channel will be filled with an infinite supply of
//    the value returned by `func`
extern Chan go(std::function<Value()> func);

[[noreturn]]
extern void stop();

extern void yield();

// Can be called by a coroutine to discover its associated channel
extern Chan me();

// Create a new channel
extern Chan chan();

// Send a value to the given channel, blocks until a receiver is present
extern void send(Chan ch, Value v);

// Receive a value from the the given channel, block until a sender
// is present
extern Value receive(Chan ch);

// convenience functions for channels

template <typename T>
inline T receive(Chan ch) {
    return std::get<T>(receive(ch));
}



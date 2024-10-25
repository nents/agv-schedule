#include <algorithm>
#include <chrono>
#include <coroutine>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

using namespace std;

#define int int64_t
#define DEBUG(x)                                                                                                       \
    do {                                                                                                               \
        auto _ = (x);                                                                                                  \
        cerr << __LINE__ << ": " << #x << " = " << _ << endl;                                                          \
    } while (0)

#define loop while (true)

struct Task {
    struct promise_type {
        Task get_return_object() { return Task{std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;

    void resume() {
        if (!handle.done()) handle.resume();
    }
    ~Task() {
        if (handle) handle.destroy();
    }
};
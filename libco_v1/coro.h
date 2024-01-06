#pragma once

#include <ucontext.h>
#include <vector>
#include <cassert>

namespace coro
{

    class coroutine;
    using func_t = void (*)(void *);

    coroutine *create(func_t func, void *args);
    void release(coroutine *co);
    int resume(coroutine *co, int param = 0);
    int yield(int ret = 0);

    struct coroutine
    {
        bool started = false;
        bool end = false;

        func_t coro_func = nullptr;
        void *args = nullptr;
        // TODO: add member variables you need
        int data;
        char *stack;
        int stack_size = 8192;

        ucontext_t ctx = {0};
        coroutine(func_t func, void *args) : coro_func(func), args(args)
        {
            stack = new char[stack_size];
                }

        ~coroutine()
        {
            if (stack != nullptr)
            {
                delete[] stack; // Deallocate stack memory
            }
        }
    };

    class coroutine_env
    {
    private:
        std::vector<coroutine *> coroutines;
        coroutine *main_coro;

    public:
        coroutine_env()
        {
            main_coro = create(nullptr, nullptr);
            push(main_coro);
        }
        ~coroutine_env()
        {
            delete main_coro;
        }

        coroutine *get_coro(int idx)
        {
            coroutine *t = coroutines[idx + 1];
            return t;
        }

        void push(coroutine *co)
        {
            coroutines.push_back(co);
        }

        void pop()
        {
            if (coroutines.size() >= 1)
            {
                coroutines.pop_back();
            }
        }
    };

} // namespace coro
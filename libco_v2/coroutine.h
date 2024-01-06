#pragma once

#include <cassert>
#include <cstdlib>
#include <vector>
#include "coro_ctx.h"

namespace coro
{

    struct coroutine;
    struct coroutine_attr;

    coroutine *create(func_t coro_func, void *arg, const coroutine_attr *attr = nullptr);
    void release(coroutine *co);

    int resume(coroutine *co, int param = 0);
    int yield(int ret = 0);

    struct stack_mem
    {
        int stack_size = 0; // 栈的大小
        coroutine *occupy_ = nullptr;
        char *bp = nullptr;
        char *sp = nullptr;

        stack_mem(size_t size) : stack_size(size)
        {
            // TODO: implement your code here
            sp = new char[size];
            bp = sp + size;
            occupy_ = nullptr;
        }

        ~stack_mem()
        {
            // TODO: implement your code here
            if (sp != NULL)
                delete[] sp;
            sp = nullptr;
        }
    };

    struct share_stack
    {
        // TODO: add member variables you need
        int count = 0;
        int stack_size = 0;
        stack_mem **stack_array = nullptr;
        int alloc = 0; // 轮询

        share_stack(int count, size_t stack_size)
            : count(count), stack_size(stack_size)
        {
            // TODO: implement your code here
            alloc = 0;
            stack_array = new stack_mem *[count];
            for (int i = 0; i < count; i++)
            {
                stack_array[i] = new stack_mem(stack_size);
            }
        }

        ~share_stack()
        {
            // TODO: implement your code here
            for (int i = 0; i < count; i++)
                delete stack_array[i];
            delete[] stack_array;
        }

        stack_mem *get_stackmem()
        {
            // TODO: implement your code here
            stack_mem *mem = stack_array[alloc];
            alloc = (alloc + 1) % count;
            return mem;
        }
    };
    struct coroutine_attr
    {
        int stack_size = 128 * 1024;
        share_stack *sstack = nullptr;
    };

    struct coroutine
    {
        bool started = false;
        bool end = false;

        func_t coro_func = nullptr;
        void *arg = nullptr;

        // TODO: add member variables you need
        stack_mem *stack = nullptr;
        context ctx = {0};
        char *save_buffer = nullptr;
        char *stack_sp = nullptr;
        int data = 0;
        int issharestack = 0;
        int save_size = 0;

        ~coroutine()
        {
            if (!issharestack)
            {
                delete stack;
            }
            else
            {
                free(save_buffer);
                if (stack->occupy_ == this)
                {
                    stack->occupy_ = nullptr;
                }
            }
        }
    };

    class coroutine_env
    {
    private:
        // TODO: add member variables you need
        coroutine *coroutines[100];

    public:
        // TODO: add member variables you need

        coroutine *occupy_co = nullptr;
        coroutine *pending = nullptr;
        int index = -1;

    public:
        ~coroutine_env()
        {
            for (int i = 0; i <= index; i++)
                delete coroutines[i];
        }
        coroutine_env()
        {
            // TODO: implement your code here
            coroutine *main_coro = create(nullptr, nullptr, nullptr);
            push(main_coro);
            index = 0;
        }
        coroutine *get_coro(int idx)
        {
            // TODO: implement your code here
            coroutine *t = coroutines[idx];
            return t;
        }

        void pop()
        {
            // TODO: implement your code here
            index--;
        }

        void push(coroutine *co)
        {
            // TODO: implement your code here
            if (index < 100)
                coroutines[++index] = co;
        }
    };
};

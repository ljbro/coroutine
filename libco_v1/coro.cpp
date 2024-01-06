#include "coro.h"

namespace coro
{

    static coroutine_env g_coro_env;

    coroutine *create(func_t func, void *args)
    {
        coroutine *temp = new coroutine(func, args);
        return temp;
    }

    void release(coroutine *co)
    {
        delete co;
    }

    static void func_wrap(coroutine *co)
    {
        if (co->coro_func)
        {
            co->coro_func(co->args);
        }
        co->end = true;
        yield(-1);
    }

    int resume(coroutine *co, int param)
    {
        coroutine *curr = g_coro_env.get_coro(-1);
        if (!co->started)
        {
            co->started = true;
            getcontext(&co->ctx);
            co->ctx.uc_stack.ss_sp = co->stack;
            co->ctx.uc_stack.ss_size = 8192;
            co->ctx.uc_link = &curr->ctx;
            makecontext(&co->ctx, (void (*)(void))func_wrap, 1, co); // 让rsp指向func_wrap
        }

        if (!co->end)
        {
            co->data = param;
            g_coro_env.push(co);
            swapcontext(&curr->ctx, &co->ctx); // 返回地址
        }

        return curr->data;
    }

    int yield(int param)
    {
        coroutine *curr = g_coro_env.get_coro(0);
        coroutine *caller = g_coro_env.get_coro(-1);
        caller->data = param;
        g_coro_env.pop();
        swapcontext(&curr->ctx, &g_coro_env.get_coro(-1)->ctx);
        return curr->data;
    }

} // namespace coro
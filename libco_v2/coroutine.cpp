#include "coroutine.h"
#include <iostream>
#include <cstring>

namespace coro
{

    static coroutine_env g_coro_env;

    extern "C"
    {
        extern void coro_ctx_swap(context *, context *) asm("coro_ctx_swap");
    };

    coroutine *create(func_t coro_func, void *arg, const coroutine_attr *attr)
    {
        coroutine_attr at;
        if (attr != nullptr)
        {
            at = *attr;
        }
        // TODO: implement your code here
        coroutine *co = new coroutine();
        co->coro_func = coro_func;
        co->arg = arg;
        if (at.stack_size < 8 * 1024)
            at.stack_size = 8 * 1024;
        else if (at.stack_size > 128 * 1024)
            at.stack_size = 128 * 1024;
        if (at.stack_size & 0xFFF)
        {
            at.stack_size &= ~0xFFF;
            at.stack_size += 0x1000;
        }

        if (at.sstack)
        {
            co->stack = at.sstack->get_stackmem();
            co->issharestack = 1;
        }
        else
        {
            co->stack = new stack_mem(128 * 1024);
            co->stack_sp = nullptr;
        }

        return co;
    }

    void release(coroutine *co)
    {

        if (!co->issharestack)
        {
            free(co->stack->sp);
            free(co->stack);
        }
        else
        {
            if (co->save_buffer)
                free(co->save_buffer);

            if (co->stack->occupy_ == co)
                co->stack->occupy_ = NULL;
        }

        free(co);
    }

    void save_stack(coroutine *co)
    {
        // TODO: implement your code here
        stack_mem *h = co->stack;
        co->save_size = h->bp - co->stack_sp;
        if (co->save_buffer)
        {
            delete[] co->save_buffer;
            co->save_buffer = NULL;
        }
        co->save_buffer = new char[co->save_size];
        memcpy(co->save_buffer, co->stack_sp, co->save_size);
    }

    void swap(coroutine *curr, coroutine *pending)
    {
        // TODO: implement your code here
        char c;
        curr->stack_sp = &c;
        if (!pending->issharestack)
        {
            g_coro_env.pending = nullptr;
            g_coro_env.occupy_co = nullptr;
        }
        else
        {
            g_coro_env.pending = pending;
            coroutine *occupyco = pending->stack->occupy_;
            pending->stack->occupy_ = pending;

            g_coro_env.occupy_co = occupyco;
            if (occupyco && occupyco != pending)
            {
                save_stack(occupyco);
            }
        }
        coro_ctx_swap(&(curr->ctx), &(pending->ctx));
        coroutine *update_occupy_co = g_coro_env.occupy_co;
        coroutine *update_pending_co = g_coro_env.pending;

        if (update_occupy_co && update_pending_co && update_occupy_co != update_pending_co)
        {
            // resume stack buffer
            if (update_pending_co->save_buffer && update_pending_co->save_size > 0)
                memcpy(update_pending_co->stack_sp, update_pending_co->save_buffer, update_pending_co->save_size);
        }
    }

    static void func_wrap(coroutine *co)
    {
        if (co->coro_func)
        {
            co->coro_func(co->arg);
        }
        co->end = true;
        yield(-1);
    }

    int resume(coroutine *co, int param)
    {
        // TODO: implement your code here
        coroutine *curr = g_coro_env.get_coro(g_coro_env.index);
        if (!co->started)
        {
            co->started = true;
            co->ctx.ss_sp = co->stack->sp;
            co->ctx.ss_size = co->stack->stack_size;
            ctx_make(&co->ctx, (void (*)(void *))func_wrap, co);
        }

        co->data = param;
        g_coro_env.push(co);
        swap(curr, co);
        return curr->data;
    }

    int yield(int ret)
    {
        // TODO: implement your code here
        coroutine *curr = g_coro_env.get_coro(g_coro_env.index);
        coroutine *caller = g_coro_env.get_coro(g_coro_env.index - 1);
        caller->data = ret;
        g_coro_env.pop();
        swap(curr, caller);
        return curr->data;
    }
} // namespace coro

#include "coro_ctx.h"

#include <cstdint>
#include <cstring>
enum
{
    kRDI = 7,
    kRSI = 8,
    kRETAddr = 9,
    kRSP = 13,
};

namespace coro
{

    void ctx_make(context *ctx, func_t coro_func, const void *arg)
    {
        // TODO: implement your code here

        char *sp = ctx->ss_sp + ctx->ss_size - sizeof(void *);
        /* align stack */
        sp = (char *)((unsigned long)sp & (-16LL)); //
        memset(ctx->regs, 0, sizeof(ctx->regs));
        void **ret_addr = (void **)(sp);
        *ret_addr = (void *)coro_func;
        ctx->regs[kRDI] = (char *)arg;
        ctx->regs[kRETAddr] = (char *)coro_func;
        ctx->regs[kRSP] = (char *)sp;
        ctx->regs[kRSI] = (char *)0;
        return;
    }

} // namespace coro

#include <cassert>
#include <iostream>

#define CO_BEGIN            \
    int tmp;                \
    switch (started)        \
    {                       \
    case 0:                 \
        started = __LINE__; \
        return a;           \
    case __LINE__:;
#define CO_END return -1;

#define CO_RETURN(value) return value
#define CO_YIELD(value) \
    started = __LINE__; \
    CO_RETURN(value);   \
    case __LINE__:;
using namespace std;
struct coroutine_base
{
    /* TODO */
    int started = 0;
};

class fib : public coroutine_base
{
private:
    /* TODO */
    int a = 0;
    int b = 1;

public:
    // TODO: update below code when you implement
    // CO_BEGIN/CO_END/CO_YIELD/CO_RETURN
    int operator()()
    {
        CO_BEGIN
        while (true)
        {
            tmp = a;
            a = b;
            b = tmp + b;
            CO_YIELD(a);
        }
    }
    CO_END
}
}
;
int main()
{
    int ans[] = {0, 1, 1, 2, 3, 5, 8, 13, 21, 34};
    fib foo;
    for (int i = 0; i < 10; i++)
        assert(foo() == ans[i]);
    std::cout << "libco_v3 test passed!" << std::endl;
}

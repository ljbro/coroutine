#include <coroutine>
#include <functional>
#include <queue>
#include <thread>
#include <iostream>

namespace coro
{
    static std::queue<std::function<bool()>> task_queue;

    struct sleep
    {
        explicit sleep(int n_ms) : delay{n_ms} {}

        bool await_ready() const noexcept
        {
            return false;
        }

        void await_suspend(std::coroutine_handle<> h)
        {
            auto start = std::chrono::steady_clock::now();

            task_queue.push([start, h, delay = delay]
                            {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > delay)
        {
            h.resume();
            return true;
        }
        return false; });
        }

        void await_resume() const noexcept
        {
        }

        std::chrono::milliseconds delay;
    };

    struct Task
    {
        struct promise_type
        {
            Task get_return_object()
            {
                return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_never final_suspend() noexcept { return {}; }
            void return_void() {}
            void unhandled_exception() {}
        };

        using handle_type = std::coroutine_handle<promise_type>;
        handle_type handle_;

        Task(handle_type handle) : handle_(handle) {}
    };

    void wait_task_queue_empty()
    {
        while (!task_queue.empty())
        {
            auto task = task_queue.front();
            if (!task())
            {
                task_queue.push(task);
            }
            task_queue.pop();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

} // namespace coro
#pragma once

#include <coroutine>
#include <iterator>
#include <utility>

namespace coro
{

    template <typename Ref, typename Value = std::remove_cvref_t<Ref>>
    class generator
    {
    public:
        // TODO: implement promise_type

        struct promise_type
        {
            Ref *value;
            generator get_return_object()
            {
                return generator(std::coroutine_handle<promise_type>::from_promise(*this));
            }
            std::suspend_never initial_suspend() { return {}; }
            std::suspend_always final_suspend() noexcept { return {}; }
            std::suspend_always yield_value(Ref &r)
            {
                value = std::addressof(r);
                return {};
            }
            void return_void() {}
            void unhandled_exception() {}
        };
        generator() noexcept = default;

        ~generator() noexcept
        {
            /* TODO */
            if (h_)
            {
                h_.destroy();
            }
        }
        struct sentinel
        {
        };
        class iterator
        {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = Value;
            using reference = Ref;
            using pointer = std::add_pointer_t<Ref>;

            iterator() noexcept = default;
            iterator(const iterator &) = delete;
            iterator(iterator &&o)
            {
                std::swap(coro_, o.coro_);
            }

            iterator &operator=(iterator &&o)
            {
                std::swap(coro_, o.coro_);
                return *this;
            }

            ~iterator() {}
            // TODO: implement operator== and operator!=
            friend bool operator==(const iterator &it, sentinel) noexcept
            {
                return it.coro_->done();
            }
            friend bool operator!=(const iterator &it, sentinel) noexcept
            {
                return !(it.coro_->done());
            }

            // TODO: implement operator++ and operator++(int)
            iterator &operator++()
            {
                if (!coro_->done())
                    coro_->resume();
                return *this;
            }
            void operator++(int)
            {
                (void)operator++();
            }
            // TODO: implement operator* and operator->
            reference operator*() const
            {
                return *((coro_->promise()).value);
            }
            reference operator->() const
            {
                return *((coro_->promise()).value);
            }

        private:
            friend generator;

            // TODO: implement iterator constructor
            // hint: maybe you need to a promise handle
            // explicit iterator() noexcept {}
            explicit iterator(std::coroutine_handle<promise_type> *ptr) noexcept : coro_{ptr} {}
            // TODO: add member variables you need
            std::coroutine_handle<promise_type> *coro_;
        };

        // TODO: implement begin() and end() member functions
        iterator begin() { return iterator(&h_); }
        sentinel end()
        {
            return {};
        }

    private:
        // TODO: implement generator constructor
        explicit generator(std::coroutine_handle<promise_type> coroutine) noexcept { h_ = coroutine; }

        // TODO: add member variables you need
        std::coroutine_handle<promise_type> h_;
    };

} // namespace coro

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
        class promise_type;
        struct promise_type
        {
        public:
            promise_type() : root_(this), leaf_(this) {}
            generator get_return_object()
            {
                return generator(std::coroutine_handle<promise_type>::from_promise(*this));
            }
            struct final_awaitable
            {
                bool await_ready() noexcept
                {
                    return false;
                }
                // std::coroutine_handle<> await_suspend(
                std::coroutine_handle<> await_suspend(std::coroutine_handle<promise_type> h) noexcept
                {
                    auto &promise = h.promise();
                    auto parent = h.promise().parent_;
                    if (parent)
                    {
                        promise.root_->leaf_ = parent;
                        return std::coroutine_handle<promise_type>::from_promise(*parent);
                    }
                    return std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            std::suspend_always initial_suspend() { return {}; }
            final_awaitable final_suspend() noexcept { return {}; }
            void return_void() noexcept {}

            std::suspend_never yield_value(Ref &&r)
            {
                root_->value_ = std::addressof(r);
                return {};
            }
            std::suspend_always yield_value(Ref &x) noexcept
            {
                root_->value_ = std::addressof(x);

                return {};
            }

            void unhandled_exception() {}
            struct yield_sequence_awaiter
            {
                generator gen_;
                // std::exception_ptr exception_;

                explicit yield_sequence_awaiter(generator &&g) noexcept
                    : gen_(std::move(g))
                {
                }

                bool await_ready() noexcept
                {
                    return !gen_.coro_;
                }

                std::coroutine_handle<promise_type> await_suspend(
                    std::coroutine_handle<promise_type> h) noexcept
                {
                    auto &current = h.promise();
                    auto &nested = gen_.coro_.promise();
                    auto &root = current.root_;

                    nested.root_ = root;
                    root->leaf_ = &nested;
                    nested.parent_ = &current;

                    return gen_.coro_;
                }
                void await_resume() noexcept {}
            };

            yield_sequence_awaiter yield_value(generator &&g) noexcept
            {
                return yield_sequence_awaiter{std::move(g)};
            }
            Ref &value() { return *root_->value_; }
            void resume()
            {
                std::coroutine_handle<promise_type>::from_promise(*leaf_).resume();
            }

            // Disable use of co_await within this coroutine.
            void await_transform() = delete;

        private:
            friend generator;

            promise_type *root_;
            promise_type *leaf_;

            promise_type *parent_ = nullptr;
            // std::exception_ptr *exception_ = nullptr;
            std::add_pointer_t<Ref> value_;
        };
        generator() noexcept = default;

        generator(generator &&other) noexcept
            : coro_(std::exchange(other.coro_, {}))
        {
        }

        ~generator() noexcept
        {
            if (coro_)
            {
                coro_.destroy();
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
                return it.coro_.done() || !it.coro_;
            }

            friend bool operator!=(const iterator &it, sentinel) noexcept
            {
                return !(it.coro_.done());
            }

            // TODO: implement operator++ and operator++(int)
            iterator &operator++()
            {
                coro_.promise().resume();
                return *this;
            }
            void operator++(int)
            {
                (void)operator++();
            }
            // TODO: implement operator* and operator->
            reference operator*() const noexcept { return static_cast<reference>(*coro_.promise().value_); }

            pointer operator->() const noexcept
                requires std::is_reference_v<reference>
            {
                return std::addressof(operator*());
            }

        private:
            friend generator;

            // TODO: implement iterator constructor
            // hint: maybe you need to a promise handle
            // explicit iterator() noexcept {}
            explicit iterator(std::coroutine_handle<promise_type> ptr) noexcept : coro_{ptr} {}
            // TODO: add member variables you need
            std::coroutine_handle<promise_type> coro_;
        };

        // TODO: implement begin() and end() member functions
        iterator begin()
        {
            if (!coro_)
                return iterator();
            auto it = iterator{coro_};
            if (!start)
            {
                ++it;
                start = true;
            }
            return it;
        }
        sentinel end()
        {
            return {};
        }

    private:
        // TODO: add member variables you need
        generator(std::coroutine_handle<promise_type> coro) noexcept : coro_(coro){};
        std::coroutine_handle<promise_type> coro_;
        bool start;
    };

} // namespace coro
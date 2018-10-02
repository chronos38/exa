#pragma once

#include <exa/task.hpp>
#include <exa/enum_flag.hpp>

#include <future>
#include <any>
#include <tuple>

namespace exa
{
    namespace detail
    {
        class io_task
        {
        public:
            template <class Result, class Function, class = std::enable_if_t<!std::is_void_v<Result>>>
            static std::future<Result> run(Function&& callback)
            {
                static_assert(std::is_invocable_v<Function>);

                auto promise = std::make_shared<std::promise<Result>>();

                task::push(std::bind(&io_task::run_internal, [callback, promise] {
                    bool done = false;
                    std::any result;

                    try
                    {
                        std::tie(done, result) = callback();

                        if (done)
                        {
                            promise->set_value(std::any_cast<Result>(result));
                            return true;
                        }
                    }
                    catch (...)
                    {
                        promise->set_exception(std::current_exception());
                        return true;
                    }

                    return false;
                }));

                return promise->get_future();
            }

            template <class Result, class Function, class = std::enable_if_t<std::is_void_v<Result>>>
            static std::future<void> run(Function&& callback)
            {
                static_assert(std::is_invocable_v<Function>);

                auto promise = std::make_shared<std::promise<void>>();
                auto first = std::make_shared<bool>(true);

                run_internal([callback, promise, first] {
                    if (*first)
                    {
                        *first = false;
                        return false;
                    }

                    try
                    {
                        if (callback())
                        {
                            promise->set_value();
                            return true;
                        }
                    }
                    catch (...)
                    {
                        promise->set_exception(std::current_exception());
                        return true;
                    }

                    return false;
                });

                return promise->get_future();
            }

        private:
            static void run_internal(std::function<bool()> callback);
        };
    }
}

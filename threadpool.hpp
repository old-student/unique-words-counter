#pragma once

#include "assert.hpp"
#include "threadpoolimpl.h"
#include <future>

class ThreadPool
{
public:
    ThreadPool(size_t numThreads)
        : tp(numThreads)
    {
        rt_assert(numThreads > 0, "invalid number of worker threads");
    }

    ~ThreadPool() = default;

    size_t numWorkers() const
    {
        return tp.numWorkers();
    }

    template <typename F>
    auto post(F&& f) -> std::future<typename std::result_of_t<F()>>
    {
        using ReturnType = typename std::result_of_t<F()>;
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
        auto res = task->get_future();
        tp.post([=](){ (*task)(); });
        return res;
    }

private:
    ThreadPoolImpl tp;
};

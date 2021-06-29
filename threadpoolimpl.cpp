#include "threadpoolimpl.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct ThreadPoolImpl::Impl
{
    Impl(size_t numWorkers)
        : numWorkers(numWorkers)
        , stopFlag(false)
    {
        for (size_t i = 0; i < numWorkers; ++i) {
            workers.emplace_back([this](){ loop(); });
        }
    }

    ~Impl()
    {
        stopFlag = true;
        cv.notify_all();
        for (auto& w : workers) {
            if (w.joinable()) { w.join(); }
        }
    }

    void loop()
    {
        for (;;) {
            Task task;
            {
                std::unique_lock lock(mtx);
                cv.wait(lock, [this](){ return stopFlag || !tasks.empty(); });
                if (stopFlag && tasks.empty()) { break; }
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }

    template <typename F>
    void post(F&& task)
    {
        {
            std::scoped_lock lock(mtx);
            tasks.push(std::forward<F>(task));
        }
        cv.notify_one();
    }

    const size_t numWorkers;
    std::atomic<bool> stopFlag;
    std::vector<std::thread> workers;
    std::queue<Task> tasks;
    std::mutex mtx;
    std::condition_variable cv;
};

ThreadPoolImpl::ThreadPoolImpl(size_t numWorkers)
    : impl(std::make_unique<Impl>(numWorkers))
{}

ThreadPoolImpl::~ThreadPoolImpl() = default;

size_t ThreadPoolImpl::numWorkers() const
{
    return impl->numWorkers;
}

void ThreadPoolImpl::post(const Task& task)
{
    impl->post(task);
}

void ThreadPoolImpl::post(Task&& task)
{
    impl->post(std::move(task));
}

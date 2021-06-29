#pragma once

#include <functional>
#include <memory>

class ThreadPoolImpl
{
    using Task = std::function<void()>;

public:
    ThreadPoolImpl(size_t);
    ~ThreadPoolImpl();

    size_t numWorkers() const;

    void post(const Task&);
    void post(Task&&);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

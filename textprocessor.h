#pragma once

#include <memory>
#include <string>

class ThreadPool;

class TextProcessor
{
public:
    TextProcessor(const std::string&, ThreadPool&);
    ~TextProcessor();

    void process();
    size_t getUniqueWordCount() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

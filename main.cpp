#include "assert.hpp"
#include "threadpool.hpp"
#include "textprocessor.h"
#include <iostream>

int main(int argc, char** argv) try
{
    rt_assert(argc > 1, "please provide input file");
    const auto numWorkers = std::thread::hardware_concurrency() > 2
        ? std::thread::hardware_concurrency() - 1 : 2;
    ThreadPool pool(numWorkers);
    TextProcessor tp(argv[1], pool);
    tp.process();
    std::cout << "Number of distinct unique words is " << tp.getUniqueWordCount() << std::endl;
    return 0;
}
catch (const std::exception& ex)
{
    std::cout << ex.what() << std::endl;
    return -1;
}
catch (...)
{
    std::cout << "exit due to unknown exception" << std::endl;
    return -1;
}

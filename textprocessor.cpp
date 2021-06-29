#include "textprocessor.h"
#include "assert.hpp"
#include "threadpool.hpp"
#include <fstream>
#include <unordered_set>
#include <shared_mutex>
#include <vector>
#include <queue>

namespace
{

constexpr size_t BLOCK_SIZE =  64 * 1024; // 64 KB

std::vector<std::string> split(const std::vector<std::string>& lines, char sep = ' ')
{
    std::vector<std::string> tokens;

    for (const auto& line : lines) {
        size_t start = 0;
        while ((start = line.find_first_not_of(sep, start)) != std::string::npos) {
            const auto end = line.find(sep, start + 1);
            if (end == std::string::npos) {
                tokens.push_back(line.substr(start));
                break;
            }
            tokens.push_back(line.substr(start, end - start));
            start = end + 1;
        }
    }

    return tokens;
}

} // namespace

struct TextProcessor::Impl
{
    using Set = std::unordered_set<std::string>;

    Impl(const std::string& fileName, ThreadPool& pool)
        : fileName(fileName)
        , pool(pool)
    {}

    void process()
    {
        std::ifstream f(fileName);
        rt_assert(f.is_open(), "error opening ", fileName);

        std::queue<std::future<Set>> fs;

        while (!f.eof()) {
            std::string line;
            std::vector<std::string> lines;
            for (size_t bytes = 0; bytes < BLOCK_SIZE && std::getline(f, line); bytes += line.size()) {
                lines.push_back(line);
            }

            if (fs.size() >= pool.numWorkers()) {
                reduce(fs);
            }

            fs.push(pool.post([ls = std::move(lines), this](){
               const auto words = split(ls);
               std::vector<std::string> v;
               v.reserve(words.size());
               {
                   std::shared_lock lock(resultSetMtx);
                   for (auto&& w : words) {
                       if (resultSet.find(w) == resultSet.cend()) {
                           v.push_back(std::move(w));
                       }
                   }
               }
               return Set(v.begin(), v.end());
            }));
        }

        reduce(fs);
    }

    void reduce(std::queue<std::future<Set>>& fs)
    {
        while (!fs.empty()) {
            auto s = fs.front().get();
            fs.pop();
            {
                std::unique_lock lock(resultSetMtx);
                resultSet.merge(s);
            }
        }
    }

    const std::string fileName;
    ThreadPool& pool;
    std::shared_mutex resultSetMtx;
    Set resultSet;
};

TextProcessor::TextProcessor(const std::string& fileName, ThreadPool& pool)
    : impl(std::make_unique<Impl>(fileName, pool))
{}

TextProcessor::~TextProcessor() = default;

void TextProcessor::process()
{
    impl->process();
}

size_t TextProcessor::getUniqueWordCount() const
{
    return impl->resultSet.size();
}

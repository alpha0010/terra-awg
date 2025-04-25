#ifndef UTIL_H
#define UTIL_H

#include <ranges>
#include <thread>
#include <vector>

/**
 * Automatic thread management for parallel loop execution.
 *
 * Before:
 * @code
 * for (int i = 0; i < 100; ++i) {
 *     handleData(i);
 * }
 * @endcode
 *
 * After:
 * @code
 * parallelFor(std::views::iota(0, 100), [](int i) {
 *     handleData(i);
 * });
 * @endcode
 */
template <std::ranges::input_range R, class UnaryFunc>
constexpr void parallelFor(R &&r, UnaryFunc f)
{
    std::vector<std::thread> pool;
    size_t numThreads = std::max(std::thread::hardware_concurrency(), 4u);
    size_t total = std::distance(r.begin(), r.end());
    size_t chunkSize = std::max<size_t>(total / numThreads, 1);
    for (size_t chunkStart = 0; chunkStart < total; chunkStart += chunkSize) {
        pool.emplace_back([&r, &f, chunkStart, chunkSize]() {
            auto itr = r.begin();
            std::advance(itr, chunkStart);
            for (size_t i = 0; i < chunkSize && itr != r.end(); ++i, ++itr) {
                f(*itr);
            }
        });
    }
    for (auto &worker : pool) {
        worker.join();
    }
}

#endif // UTIL_H

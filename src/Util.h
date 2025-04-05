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
    for (size_t loopId = 0; loopId < numThreads; ++loopId) {
        pool.emplace_back([&r, &f, loopId, numThreads]() {
            auto itr = r.begin();
            for (size_t i = 0; i < loopId && itr != r.end(); ++i, ++itr)
                ;
            while (itr != r.end()) {
                f(*itr);
                for (size_t i = 0; i < numThreads && itr != r.end(); ++i, ++itr)
                    ;
            }
        });
    }
    for (auto &worker : pool) {
        worker.join();
    }
}

#endif // UTIL_H

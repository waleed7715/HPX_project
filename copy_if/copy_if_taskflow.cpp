#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp>

#include "Random.hpp"
#include "Helper.hpp"

#ifdef HAVE_VTUNE
#include <ittnotify.h>
#endif

int main(int argc, char* argv[])
{
    int input_size = 100'000;
    int threads    = 1;
    int runs       = 1;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if      (arg.rfind("--input_size=", 0) == 0) input_size = std::stoi(arg.substr(13));
        else if (arg.rfind("--threads=",    0) == 0) threads    = std::stoi(arg.substr(10));
        else if (arg.rfind("--runs=",       0) == 0) runs       = std::stoi(arg.substr(7));
    }

    tf::Executor executor(threads);

    std::string filename = "test_data_" + std::to_string(input_size) + ".bin";
    std::vector<int> source = load_vector(filename);
    const size_t n          = source.size();
    const size_t chunk_size = (n + static_cast<size_t>(threads) - 1)
                              / static_cast<size_t>(threads);

    std::vector<uint8_t> flags(n);
    std::vector<size_t>  local_counts(threads, 0);
    std::vector<size_t>  offsets(threads, 0);
    std::vector<int>     destination(n);

    for (int r = 0; r < runs; ++r) {

        size_t total_count = 0;
        tf::Taskflow taskflow;

        #ifdef HAVE_VTUNE
        __itt_resume();
        #endif

        auto start = std::chrono::high_resolution_clock::now();

        // Phase 1 (F1): evaluate predicate
        auto f1 = taskflow.for_each_index(
            size_t(0), static_cast<size_t>(threads), size_t(1),
            [&](size_t chunk_id) {
                const size_t lo = chunk_id * chunk_size;
                const size_t hi = std::min(lo + chunk_size, n);
                size_t cnt = 0;
                for (size_t i = lo; i < hi; ++i) {
                    uint8_t v = Pred(source[i]) ? 1u : 0u;
                    flags[i]  = v;
                    cnt       += v;
                }
                local_counts[chunk_id] = cnt;
            }
        );

        // Phase 2 (F2): sequential prefix sum
        auto f2 = taskflow.emplace([&]() {
            size_t running = 0;
            for (int i = 0; i < threads; ++i) {
                offsets[i] = running;
                running   += local_counts[i];
            }
            total_count = running;
        });

        // Phase 3 (F3): scatter
        auto f3 = taskflow.for_each_index(
            size_t(0), static_cast<size_t>(threads), size_t(1),
            [&](size_t chunk_id) {
                const size_t lo  = chunk_id * chunk_size;
                const size_t hi  = std::min(lo + chunk_size, n);
                size_t out = offsets[chunk_id];
                for (size_t i = lo; i < hi; ++i) {
                    if (flags[i])
                        destination[out++] = source[i];
                }
            }
        );

        f1.precede(f2);
        f2.precede(f3);

        executor.run(taskflow).wait();

        auto end = std::chrono::high_resolution_clock::now();

        #ifdef HAVE_VTUNE
        __itt_pause();
        #endif

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << input_size
            << ", " << threads
            << ", " << 0
            << ", " << total_count
            << ", " << duration << "\n";
    }

    return 0;
}

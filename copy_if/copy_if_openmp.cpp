#include <omp.h>

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

    omp_set_num_threads(threads);

    std::string filename = "test_data_" + std::to_string(input_size) + ".bin";
    std::vector<int> source = load_vector(filename);
    const size_t n = source.size();

    std::vector<uint8_t> flags(n);
    std::vector<size_t>  local_counts(threads, 0);
    std::vector<size_t>  offsets(threads, 0);
    std::vector<int>     destination(n);

    #pragma omp parallel
    {
        const int    tid   = omp_get_thread_num();
        const int    nthr  = omp_get_num_threads();
        const size_t chunk = (n + nthr - 1) / nthr;
        const size_t lo    = static_cast<size_t>(tid) * chunk;
        const size_t hi    = std::min(lo + chunk, n);
        for (size_t i = lo; i < hi; ++i)
            destination[i] = 0;
    }

    for (int r = 0; r < runs; ++r) {

        size_t total_count = 0;

        #ifdef HAVE_VTUNE
        __itt_resume();
        #endif

        auto start = std::chrono::high_resolution_clock::now();

        #pragma omp parallel shared(total_count)
        {
            const int    tid   = omp_get_thread_num();
            const int    nthr  = omp_get_num_threads();
            const size_t chunk = (n + nthr - 1) / nthr;
            const size_t lo    = static_cast<size_t>(tid) * chunk;
            const size_t hi    = std::min(lo + chunk, n);

            // Phase 1 (F1): evaluate predicate
            size_t cnt = 0;
            for (size_t i = lo; i < hi; ++i) {
                uint8_t v = Pred(source[i]) ? 1u : 0u;
                flags[i]  = v;
                cnt       += v;
            }
            local_counts[tid] = cnt;

            // Barrier
            #pragma omp barrier

            // Phase 2 (F2): sequential prefix sum
            #pragma omp single
            {
                size_t running = 0;
                for (int i = 0; i < nthr; ++i) {
                    offsets[i] = running;
                    running   += local_counts[i];
                }
                total_count = running;
            }

            // Phase 3 (F3): scatter matching elements to their output positions
            size_t out = offsets[tid];
            for (size_t i = lo; i < hi; ++i) {
                if (flags[i])
                    destination[out++] = source[i];
            }
        }

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

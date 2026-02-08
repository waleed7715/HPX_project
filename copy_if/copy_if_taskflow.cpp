#include <taskflow/taskflow.hpp>
#include <taskflow/algorithm/for_each.hpp> 

#include "Random.hpp"
#include "Helper.hpp"

#ifdef HAVE_VTUNE
#include <ittnotify.h>
#endif

void copy_if_taskflow(const std::vector<int>& source, int threads)
{
    tf::Executor executor(threads);
    tf::Taskflow taskflow;

    const size_t n = source.size();

    std::vector<bool> flags(n, false);
    std::vector<size_t> local_counts(threads, 0);
    const size_t chunk_size = (n + threads - 1) / threads;
    std::vector<int> destination(n);

    #ifdef HAVE_VTUNE
    __itt_resume();
    #endif

    auto start = std::chrono::high_resolution_clock::now();

    taskflow.for_each_index(
        size_t(0), size_t(threads), size_t(1),
        [&](size_t thread_id)
        {
            const size_t start_idx = thread_id * chunk_size;
            const size_t end_idx = std::min(start_idx + chunk_size, n);
            size_t local_count = 0;

            for (size_t i = start_idx; i < end_idx; ++i)
            {
                flags[i] = Pred(source[i]);
                if (flags[i])
                {
                    ++local_count;
                }
            }

            local_counts[thread_id] = local_count;
        }
    );

    executor.run(taskflow).wait();
    taskflow.clear();

    std::vector<size_t> offsets(threads);
    offsets[0] = 0;
    for (size_t i = 1; i < threads; ++i)
    {
        offsets[i] = offsets[i - 1] + local_counts[i - 1];
    }

    size_t total_count = offsets[threads - 1] + local_counts[threads - 1];

    taskflow.for_each_index(
        size_t(0), size_t(threads), size_t(1),
        [&](size_t thread_id)
        {
            const size_t start_idx = thread_id * chunk_size;
            const size_t end_idx = std::min(start_idx + chunk_size, n);

            size_t write_idx = offsets[thread_id];

            for (size_t i = start_idx; i < end_idx; ++i)
            {
                if (flags[i])
                {
                    destination[write_idx++] = source[i];
                }
            }
        }
    );

    executor.run(taskflow).wait();

    auto end = std::chrono::high_resolution_clock::now();

    #ifdef HAVE_VTUNE
    __itt_pause();
    #endif

    destination.resize(total_count);

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Size: " << n
        << ",  Threads: " << threads
        << ", Copied elements: " << destination.size() 
        << ", Duration: " << duration << " us\n";
}

int main(int argc, char* argv[])
{
    #ifdef HAVE_VTUNE
    __itt_pause();
    #endif

    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };
    std::vector<int> num_threads{ 1, 2, 4, 8, 16 };

    for (auto size: vector_size)
    {
        std::cout << "Source size: " << size << "\n";

        const size_t n = static_cast<size_t>(size);

        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);

        for (auto threads: num_threads)
        {
            copy_if_taskflow(source, threads);
        }
    }
    return 0;
}

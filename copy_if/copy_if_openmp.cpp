#include <omp.h>

#include "Random.hpp"
#include "Helper.hpp"

void copy_if_openmp(const std::vector<int>& source, int threads)
{
    omp_set_num_threads(threads);
    
    const size_t n = source.size();

    std::vector<bool> flags(n);
    std::vector<size_t> local_counts(threads, 0);
    std::vector<size_t> offsets(threads);

    auto start = std::chrono::high_resolution_clock::now();

    #pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        const int n_threads = omp_get_num_threads();

        const size_t chunk_size = (n + n_threads - 1) / n_threads;
        const size_t start_idx = thread_id * chunk_size;
        const size_t end_idx = std::min(start_idx + chunk_size, n);

        size_t local_count = 0;
        for (size_t i = start_idx; i < end_idx; ++i)
        {
            if (source[i] % 7 == 0)
            {
                flags[i] = true;
                ++local_count;
            }
        }

        local_counts[thread_id] = local_count;
    }

    size_t total_count = 0;
    for (size_t i = 0; i < local_counts.size(); ++i)
    {
        offsets[i] = total_count;
        total_count += local_counts[i];
    }   

    std::vector<int> destination(total_count);

    #pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        const int n_threads = omp_get_num_threads();

        const size_t chunk_size = (n + n_threads - 1) / n_threads;
        const size_t start_idx = thread_id * chunk_size;
        const size_t end_idx = std::min(start_idx + chunk_size, n);

        size_t local_offset = offsets[thread_id];

        for (size_t i = start_idx; i < end_idx; ++i)
        {
            if (flags[i])
            {
                destination[local_offset++] = source[i];
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "Size: " << n
        << ",  Threads: " << threads
        << ", Copied elements: " << destination.size() 
        << ", Duration: " << duration << " us\n";
}

int main(int argc, char* argv[])
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };
    std::vector<int> num_threads{ 1, 2, 4, 8, 16 };

    for (auto size: vector_size)
    {
        const size_t n = static_cast<size_t>(size);

        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);
        
        std::cout << "Source size: " << source.size() << "\n";

        for (auto threads: num_threads)
        {
            copy_if_openmp(source, threads);
        }
    }

    return 0;
}

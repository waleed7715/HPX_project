#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include "Random.hpp"

int hpx_main()
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };
    std::vector<int> num_threads{ 1, 2, 4, 8, 16, 32 };

    for (auto size: vector_size)
    {
        std::vector<int> source(size);

        hpx::generate(hpx::execution::par, source.begin(), source.end(),
            [size] { return Random::get(0, static_cast<int>(size - 1)); });

        std::cout << "Source size: " << source.size() << "\n";

        for (auto threads: num_threads)
        {
            auto start = std::chrono::high_resolution_clock::now();

            size_t chunk_size = (size + threads - 1) / threads;

            std::vector<hpx::future<std::vector<int>>> futures;
            futures.reserve(threads);

            for (size_t i = 0; i < threads; ++i)
            {
                size_t start_index = i * chunk_size;
                size_t end_index = std::min<size_t>(start_index + chunk_size, static_cast<size_t>(size));

                futures.push_back(hpx::async([chunk_size, start_index, end_index, &source]() {
                    std::vector<int> local_destination;
                    local_destination.reserve(chunk_size);

                    for (size_t j = start_index; j < end_index; ++j)
                    {
                        if (source[j] % 2 == 0)
                        {
                            local_destination.push_back(source[j]);
                        }
                    }
                    return local_destination;
                }));
            }

        hpx::wait_all(futures);

        std::vector<std::vector<int>> chunks;
        chunks.reserve(futures.size());

        size_t total_size = 0;
        for (auto& fut : futures)
        {
            chunks.push_back(fut.get());
            total_size += chunks.back().size();
        }

        std::vector<int> destination;
        destination.reserve(total_size);

        for (auto& chunk : chunks)
        {
            destination.insert(destination.end(),
                            std::make_move_iterator(chunk.begin()),
                            std::make_move_iterator(chunk.end()));
        }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            std::cout << "Size: " << size
                << ",  Threads: " << threads
                << ", Copied elements: " << destination.size() 
                << ", Duration: " << duration << " us\n";
        }
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}

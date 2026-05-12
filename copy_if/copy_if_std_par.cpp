#include <tbb/tbb.h>
#include <execution>
#include <algorithm>
#include <chrono>
#include <vector>
#include <iostream>

#include "Random.hpp"
#include "Helper.hpp"

int main(int argc, char* argv[])
{
    int input_size = (argc > 1) ? std::stoi(argv[1]) : 100'000;

    std::vector<int> vector_size{ input_size };
    std::vector<int> num_threads{ 1, 2, 4, 8, 12, 16, 24};

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);

        for (auto threads : num_threads)
        {
            tbb::global_control gc(
                tbb::global_control::max_allowed_parallelism, threads);

            std::vector<int> destination(size);

            auto start = std::chrono::high_resolution_clock::now();

            auto end_it = std::copy_if(std::execution::par,
                source.begin(),
                source.end(),
                destination.begin(),
                Pred<int>
            );

            auto end = std::chrono::high_resolution_clock::now();

            destination.resize(std::distance(destination.begin(), end_it));

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - start).count();

            std::cout << size
                << ", " << threads
                << ", " << destination.size()
                << ", " << duration << "\n";
        }
    }

    return 0;
}

// std_copy_if.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Uses MSVC 19.50 with Microsoft PPL, not intel TBB

#include "Random.hpp"
#include "Helper.hpp"

int main()
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };

    for (auto size : vector_size)
    {
        const size_t n = static_cast<size_t>(size);

        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);
        
        std::cout << "Source size: " << source.size() << "\n";
        
        std::vector<int> dest(n);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto end_it = std::copy_if(std::execution::par, source.begin(), source.end(),
            dest.begin(),
            [](int elem) { return elem % 7 == 0; });

        auto end = std::chrono::high_resolution_clock::now();

        dest.resize(std::distance(dest.begin(), end_it));

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();

        std::cout << "Size: " << n
            << ", Copied elements: " << dest.size() 
            << ", Duration: " << duration << " us\n";
    }

    return 0;
}

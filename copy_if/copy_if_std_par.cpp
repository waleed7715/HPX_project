// std_copy_if.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <algorithm>
#include <iostream>
#include <vector>
#include <execution>
#include <chrono>

#include "Random.hpp"

int main()
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };

    for (auto size : vector_size)
    {
        std::vector<int> source(size);

        std::generate(std::execution::par, source.begin(), source.end(),
            [size] { return Random::get(0, static_cast<int>(size - 1)); });
        
        // Memory bound
        {
            std::vector<int> dest(size);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            auto end_it = std::copy_if(std::execution::par, source.begin(), source.end(),
                dest.begin(),
				[](int elem) { return elem % 7 == 0; });

            auto end = std::chrono::high_resolution_clock::now();

			dest.resize(std::distance(dest.begin(), end_it));

            std::chrono::duration<double> elapsed = end - start;
            
            std::cout << "Source size: " << source.size() << ", Dest size: " << dest.size() 
                << ", Time: " << elapsed << "\n";
        }

		// CPU bound
        {
            std::vector<int> dest(size);

            auto start = std::chrono::high_resolution_clock::now();

            auto end_it = std::copy_if(std::execution::par, source.begin(), source.end(),
                dest.begin(),
                [](int elem)
                {
                    if (elem < 2) return false;

                    if (elem == 2) return true;

                    if (elem % 2 == 0) return false;

                    for (int i = 3; i * i <= elem; i += 2)
                    {
                        if (elem % i == 0) return false;
                    }

                    return true;
                });

            auto end = std::chrono::high_resolution_clock::now();

			dest.resize(std::distance(dest.begin(), end_it));

            std::chrono::duration<double> elapsed = end - start;

            std::cout << "Source size: " << source.size() << ", Dest size: " << dest.size()
                << ", Time: " << elapsed << "\n";
        }
    }

    return 0;
}

// std_copy_if.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <algorithm>
#include <iostream>
#include <vector>
#include <execution>
#include <chrono>
#include <fstream>

#include "Random.hpp"

std::vector<int> load_vector(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    std::vector<int> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size * sizeof(int));
    
    return data;
}

int main()
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);
        
        std::cout << "Source size: " << source.size() << "\n";
        
        // Memory bound
        {
            std::vector<int> dest(size);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            auto end_it = std::copy_if(std::execution::seq, source.begin(), source.end(),
                dest.begin(),
				[](int elem) { return elem % 7 == 0; });

            auto end = std::chrono::high_resolution_clock::now();

			dest.resize(std::distance(dest.begin(), end_it));

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            
            std::cout << "Size: " << size
                << ", Copied elements: " << dest.size() 
                << ", Duration: " << duration << " us\n";
        }

		// CPU bound
        // {
        //     std::vector<int> dest(size);

        //     auto start = std::chrono::high_resolution_clock::now();

        //     auto end_it = std::copy_if(std::execution::seq, source.begin(), source.end(),
        //         dest.begin(),
        //         [](int elem)
        //         {
        //             if (elem < 2) return false;

        //             if (elem == 2) return true;

        //             if (elem % 2 == 0) return false;

        //             for (int i = 3; i * i <= elem; i += 2)
        //             {
        //                 if (elem % i == 0) return false;
        //             }

        //             return true;
        //         });

        //     auto end = std::chrono::high_resolution_clock::now();

		// 	dest.resize(std::distance(dest.begin(), end_it));

        //     std::chrono::duration<double> elapsed = end - start;

        //     std::cout << "Source size: " << source.size() << ", Dest size: " << dest.size()
        //         << ", Time: " << elapsed << "\n";
        // }
    }

    return 0;
}

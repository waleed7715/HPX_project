// generate_data.cpp - Run this once to create test data
#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>
#include "Random.hpp"

void save_vector(const std::string& filename, const std::vector<int>& data)
{
    std::ofstream file(filename, std::ios::binary);
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(reinterpret_cast<const char*>(data.data()), size * sizeof(int));
}

int main()
{
    std::vector<int> sizes{ 100'000, 10'000'000, 1'000'000'000 };
    
    for (auto size : sizes)
    {
        std::vector<int> data(size);
        std::generate(data.begin(), data.end(),
            [size] { return Random::get(0, static_cast<int>(size - 1)); });
        
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        save_vector(filename, data);
        std::cout << "Generated " << filename << "\n";
    }
    
    return 0;
}

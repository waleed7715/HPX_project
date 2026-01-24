#include <algorithm>
#include <iostream>
#include <vector>

#include "Random.hpp"

int main()
{
    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };

    for (auto size: vector_size)
    {
        std::vector<int> source(size);

        std::generate(source.begin(), source.end(),
            [size] { return Random::get(0, static_cast<int>(size - 1)); });

        std::cout << "Source size: " << source.size() << "\n";
    }

    return 0;
}

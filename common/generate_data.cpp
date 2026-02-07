#include "Random.hpp"
#include "Helper.hpp"

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

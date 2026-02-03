#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include "Random.hpp"
#include "Helper.hpp"

int hpx_main()
{
    int threads = hpx::get_num_worker_threads();
    
    std::cout << "HPX Configuration (CLEAN VERSION):\n";
    std::cout << "  OS threads: " << hpx::get_os_thread_count() << "\n";
    std::cout << "  Worker threads: " << threads << "\n\n";

    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);
        
        std::cout << "Source size: " << source.size() << "\n";

        std::vector<int> destination(size);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto end_it = hpx::copy_if(hpx::execution::par, 
            source.begin(), source.end(), destination.begin(),
            [](int elem) { return elem % 7 == 0; });

        auto end = std::chrono::high_resolution_clock::now();

        destination.resize(std::distance(destination.begin(), end_it));

        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
            end - start).count();
        
        std::cout << "Size: " << size
            << ",  Threads: " << threads
            << ", Copied: " << destination.size() 
            << ", Duration: " << duration << " us\n";
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    return hpx::init(argc, argv);
}

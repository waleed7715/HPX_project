#include <algorithm>
#include <iostream>
#include <vector>
#include <chrono>
#include <fstream>

#ifdef HAVE_VTUNE
#include <ittnotify.h>
#endif

#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/program_options.hpp>

#include "Random.hpp"

std::vector<int> load_vector(const std::string& filename)
{
    #ifdef HAVE_VTUNE
    __itt_pause();
    #endif
 
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));
    
    std::vector<int> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size * sizeof(int));
    
    #ifdef HAVE_VTUNE
    __itt_resume();
    #endif
    
    return data;
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    int req_cores = vm["num-cores"].as<int>();
    
    if (req_cores <= 0) {
        req_cores = hpx::get_num_worker_threads();
    }

    std::cout << "HPX Configuration:\n";
    std::cout << "  OS threads: " << hpx::get_os_thread_count() << "\n";
    std::cout << "  Worker threads: " << hpx::get_num_worker_threads() << "\n";
    std::cout << "  Requested num_cores: " << req_cores << "\n\n";

    std::vector<int> vector_size{ 100'000, 10'000'000, 1'000'000'000 };
    std::vector<int> num_threads{ req_cores };

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);
        
        std::cout << "Source size: " << source.size() << "\n";

        for (auto threads : num_threads)
        {
            hpx::execution::experimental::num_cores n_cores(threads);

            std::vector<int> destination(size);
            
            auto start = std::chrono::high_resolution_clock::now();
            
            auto end_it = hpx::copy_if(hpx::execution::par.with(n_cores), 
                source.begin(), source.end(), destination.begin(),
                [](int elem) { return elem % 7 == 0; });

            auto end = std::chrono::high_resolution_clock::now();

            destination.resize(std::distance(destination.begin(), end_it));

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - start).count();
            
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
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");
    
    desc_commandline.add_options()
        ("num-cores",
            hpx::program_options::value<int>()->default_value(0),
            "Number of cores for num_cores (0 = use all HPX worker threads)")
        ;
    
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    
    return hpx::init(hpx_main, argc, argv, init_args);
}

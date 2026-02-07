#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/program_options.hpp>

#include "Random.hpp"
#include "Helper.hpp"

int hpx_main(hpx::program_options::variables_map& vm)
{
    int req_cores = vm["num-cores"].as<int>();
    
    if (req_cores <= 0) {
        req_cores = hpx::get_num_worker_threads();
    }

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
            
            auto start = std::chrono::high_resolution_clock::now();
            
            auto it = hpx::find_if(hpx::execution::par.with(n_cores), 
                source.begin(), 
                source.end(), 
                Pred<int>
            );

            auto end = std::chrono::high_resolution_clock::now();

            if (it != source.end()) {
                std::cout << "Found value: " << *it << "\n";
            } else {
                std::cout << "Value not found.\n";
            }

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - start).count();
            
            std::cout << "Size: " << size
                << ",  Threads: " << threads
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

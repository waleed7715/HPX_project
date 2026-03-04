#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>

#include "Random.hpp"
#include "Helper.hpp"

int hpx_main(hpx::program_options::variables_map& vm)
{
    int input_size = vm["input_size"].as<int>();

    int threads = hpx::get_num_worker_threads();

    std::vector<int> vector_size{ input_size };

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);

        std::vector<int> destination(size);
        
        auto start = std::chrono::high_resolution_clock::now();
        
        auto end_it = hpx::copy_if(hpx::execution::par, 
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
            << ", " <<threads
            << ", " << destination.size() 
            << ", " << duration << "\n";
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");
    
    desc_commandline.add_options()
        ("input_size",
            hpx::program_options::value<int>()->default_value(100'000),
            "Sizes of input vector (default: 100000)")
        ;
    
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    
    return hpx::init(hpx_main, argc, argv, init_args);
}

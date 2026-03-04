#include <hpx/hpx.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/program_options.hpp>
#include <hpx/threading_base/scoped_annotation.hpp>

#ifdef HAVE_VTUNE
#include <ittnotify.h>
#endif

#include "Random.hpp"
#include "Helper.hpp"

int hpx_main(hpx::program_options::variables_map& vm)
{
    int hpx_threads = vm["hpx_threads"].as<int>();
    int input_size = vm["input_size"].as<int>();
    int num_chunks = vm["num_chunks"].as<int>();

    if (hpx_threads <= 0) {
        hpx_threads = hpx::get_num_worker_threads();
    }

    if (input_size <= 0) {
        input_size = 100'000;
    }

    std::vector<int> vector_size{ input_size };
    std::vector<int> num_threads{ hpx_threads };

    for (auto size : vector_size)
    {
        std::string filename = "test_data_" + std::to_string(size) + ".bin";
        std::vector<int> source = load_vector(filename);

        for (auto threads : num_threads)
        {
            hpx::execution::experimental::num_cores n_cores(threads);

            // Define number of chunks
            std::size_t chunk_size = 0;
            if (num_chunks > 0) {
                std::size_t total_chunks = threads * num_chunks;
                chunk_size = (size + total_chunks - 1) / total_chunks;
            }

            auto run = [&](auto exec_policy) {
                std::vector<int> destination(size);
                
                auto start = std::chrono::high_resolution_clock::now();
                
                auto end_it = hpx::copy_if(exec_policy,
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
            };

            if (chunk_size > 0) {
                run(hpx::execution::par.with(n_cores, hpx::execution::static_chunk_size(chunk_size)));
            }
            else {
                run(hpx::execution::par.with(n_cores));
            }
        }
    }

    return hpx::finalize();
}

int main(int argc, char* argv[])
{
    hpx::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");
    
    desc_commandline.add_options()
        ("hpx_threads",
            hpx::program_options::value<int>()->default_value(0),
            "Number of HPX threads (0 = default)")
        ("input_size",
            hpx::program_options::value<int>()->default_value(100'000),
            "Sizes of input vector (default: 100000)")
        ("num_chunks",
            hpx::program_options::value<int>()->default_value(0),
            "Number of chunks (0 = default)")
        ;
    
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    
    return hpx::init(hpx_main, argc, argv, init_args);
}

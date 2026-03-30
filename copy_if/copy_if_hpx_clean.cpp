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
    int input_size = vm["input_size"].as<int>();
    int num_chunks = vm["num_chunks"].as<int>();
    int iterations = vm["iterations"].as<int>();

    int threads = hpx::get_num_worker_threads();

    if (input_size <= 0) {
        input_size = 100'000;
    }

    for (int i = 0; i < iterations; ++i) {
        std::string filename = "test_data_" + std::to_string(input_size) + ".bin";
        std::vector<int> source = load_vector(filename);

        // Define number of chunks
        std::size_t chunk_size = 0;
        if (num_chunks > 0) {
            std::size_t total_chunks = threads * num_chunks;
            chunk_size = (input_size + total_chunks - 1) / total_chunks;
        }

        auto run = [&](auto exec_policy) {
            std::vector<int> destination(input_size);

            #if HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
                static hpx::util::itt::event ts("TIMER_START");
                hpx::util::itt::event_tick(ts);
            #endif
            
            auto start = std::chrono::high_resolution_clock::now();
            
            auto end_it = hpx::copy_if(exec_policy,
                source.begin(), 
                source.end(), 
                destination.begin(), 
                Pred<int>
            );
            
            auto end = std::chrono::high_resolution_clock::now();

            #if HPX_HAVE_ITTNOTIFY != 0 && !defined(HPX_HAVE_APEX)
                static hpx::util::itt::event e("TIMER_END");
                hpx::util::itt::event_tick(e);
            #endif
            
            destination.resize(std::distance(destination.begin(), end_it));

            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                end - start).count();
            
            std::cout << input_size
                << ", " <<threads
                << ", " << num_chunks
                << ", " << destination.size() 
                << ", " << duration << "\n";
        };

        if (chunk_size > 0) {
            run(hpx::execution::par.with(hpx::execution::static_chunk_size(chunk_size)));
        }
        else {
            run(hpx::execution::par);
        }
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
        ("num_chunks",
            hpx::program_options::value<int>()->default_value(0),
            "Number of chunks (0 = default)")
        ("iterations",
            hpx::program_options::value<int>()->default_value(1),
            "Number of iterations (default: 1)")
        ;
    
    hpx::init_params init_args;
    init_args.desc_cmdline = desc_commandline;
    
    return hpx::init(hpx_main, argc, argv, init_args);
}

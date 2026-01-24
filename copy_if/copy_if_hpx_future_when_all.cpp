#include <hpx/algorithm.hpp>
#include <hpx/hpx_init.hpp>
#include <hpx/future.hpp>

#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include "Random.hpp"

int hpx_main(hpx::program_options::variables_map& vm)
{
    std::size_t size = vm["size"].as<std::size_t>();

    // Generate random data
    std::vector<int> source(size);    
    std::generate(source.begin(), source.end(), 
        [size] { return Random::get(0, static_cast<int>(size-1)); });
    
    std::vector<int> evens(size);
    std::vector<int> odds(size);

    auto start = std::chrono::high_resolution_clock::now();
    
    // Launch both copy_if operations asynchronously
    auto fut_evens = hpx::copy_if(
        hpx::execution::par(hpx::execution::task),
        source.begin(), source.end(),
        evens.begin(),
        [](int x) { return x % 2 == 0; }
    );
    
    auto fut_odds = hpx::copy_if(
        hpx::execution::par(hpx::execution::task),
        source.begin(), source.end(),
        odds.begin(),
        [](int x) { return x % 2 != 0; }
    );
    
    // Wait for both and resize
    hpx::when_all(fut_evens, fut_odds).then(
        [&](auto fut) {
            auto results = fut.get();
            auto end_evens = std::get<0>(results).get();
            auto end_odds = std::get<1>(results).get();
            
            evens.resize(std::distance(evens.begin(), end_evens));
            odds.resize(std::distance(odds.begin(), end_odds));
        }
    ).get();
    
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Output results
    std::cout << "Size: " << size << "\n";
    std::cout << "Time: " << elapsed.count() << " seconds\n";
    std::cout << "Evens: " << evens.size() << "\n";
    std::cout << "Odds: " << odds.size() << "\n";
    
    // Display vectors only for small inputs
    if (size <= 50) {
        std::cout << "\nSource: ";
        for (auto v : source) std::cout << v << " ";
        std::cout << "\n\nEvens: ";
        for (auto v : evens) std::cout << v << " ";
        std::cout << "\n\nOdds: ";
        for (auto v : odds) std::cout << v << " ";
        std::cout << "\n";
    }
        
    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    using namespace hpx::program_options;
    
    options_description cmdline("usage: " HPX_APPLICATION_STRING " [options]");

    cmdline.add_options()
        ("size,s", value<std::size_t>()->default_value(100),
         "size of the vector to test");
    
    hpx::local::init_params init_args;
    init_args.desc_cmdline = cmdline;

    return hpx::local::init(hpx_main, argc, argv, init_args);
}

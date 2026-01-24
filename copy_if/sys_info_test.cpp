#include <hpx/hpx_init.hpp>
#include <iostream>

int hpx_main() {
    std::cout << "Worker threads: " << hpx::get_num_worker_threads() << "\n";
    std::cout << "HW concurrency: " << hpx::threads::hardware_concurrency() << "\n";
    
    return hpx::finalize();
}

int main(int argc, char* argv[]) {
    return hpx::init(argc, argv);
}

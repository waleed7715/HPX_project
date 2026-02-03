#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <execution>
#include <chrono>
#include <string>
#include <fstream>

#ifdef HAVE_VTUNE
#include <ittnotify.h>
#endif

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

void save_vector(const std::string& filename, const std::vector<int>& data)
{
    std::ofstream file(filename, std::ios::binary);
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(reinterpret_cast<const char*>(data.data()), size * sizeof(int));
}

#endif

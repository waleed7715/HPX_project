#ifndef HELPER_HPP
#define HELPER_HPP

#include <algorithm>
#include <iostream>
#include <vector>
#include <execution>
#include <chrono>
#include <string>
#include <fstream>
#include <stdexcept>

#ifndef COMMON_DATA_DIR
#define COMMON_DATA_DIR "."
#endif

inline std::vector<int> load_vector(const std::string& filename)
{
    const std::string full_path =
        std::string(COMMON_DATA_DIR) + "/" + filename;

    std::ifstream file(full_path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + full_path);
    }

    size_t size;
    file.read(reinterpret_cast<char*>(&size), sizeof(size));

    std::vector<int> data(size);
    file.read(reinterpret_cast<char*>(data.data()), size * sizeof(int));

    return data;
}

inline void save_vector(const std::string& filename,
                        const std::vector<int>& data)
{
    std::ofstream file(filename, std::ios::binary);
    size_t size = data.size();
    file.write(reinterpret_cast<const char*>(&size), sizeof(size));
    file.write(reinterpret_cast<const char*>(data.data()), size * sizeof(int));
}

template <typename T>
bool Pred(const T& value)
{
    return value % 42 == 0;
}

#endif

#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <filesystem>
#include <fstream>
#include <string>

namespace sl {
    
std::string read_file(const std::filesystem::path& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    size_t file_size = static_cast<size_t>(file.tellg());
    std::string buffer;
    buffer.reserve(file_size);
    file.seekg(0);
    size_t counter = 0;
    while (counter++ != file_size) buffer += file.get();
    file.close();
    return buffer;
}

} // namespace sl

#endif
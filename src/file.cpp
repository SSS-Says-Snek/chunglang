#include <fstream>
#include <sstream>

#include "chung/file.hpp"

std::string read_source(const std::string& file_path) {
    std::ifstream file{file_path};
    std::stringstream content_buffer;

    if (file.is_open()) {
        content_buffer << file.rdbuf();
    }

    return content_buffer.str();
}

// ... kind of. It just checks for accessability
bool file_exists(const std::string& file_path) {
    std::ifstream file{file_path};
    return file.good();
}
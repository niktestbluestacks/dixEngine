#ifndef DIX_RANDOM_HPP
#define DIX_RANDOM_HPP

// dix
#include <Utils/Converter.hpp>

// std
#include <cstdint>
#include <filesystem>
#include <vector>

namespace dix {
namespace fs = std::filesystem;
inline std::string getRandomFile(std::string filepath) {
    if (filepath.back() == '/') filepath.pop_back();

    std::vector <std::filesystem::path> files (0);
    for (auto const& entry : fs::directory_iterator(filepath)) {
        if (fs::is_regular_file(entry.status())) files.push_back(entry.path());
    }

    std::srand(static_cast<unsigned>(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::size_t idx = static_cast<std::size_t>(std::rand()) % files.size();

    return files[idx].string();
}
}

#endif // DIX_RANDOM_HPP
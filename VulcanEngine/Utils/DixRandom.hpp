#ifndef DIX_RANDOM_HPP
#define DIX_RANDOM_HPP

// dix
#include <Utils/Converter.hpp>
#include <Utils/DixFilesystem.hpp>

// std
#include <cstdlib>
#include <vector>
#include <random>

namespace dix {
inline std::string getRandomFile(std::string filepath = toAudioPath("")) {
    static std::random_device rd;
    static std::mt19937 gen{rd()};
    if (filepath.back() == '/') filepath.pop_back();

    std::vector<fs::path> files(0);
    for (const auto& entry : fs::directory_iterator{filepath}) {
        if (fs::is_regular_file(entry.status())) files.push_back(entry.path());
    }
    std::uniform_int_distribution<std::size_t> dist{0, files.size() - 1};
    std::size_t idx = dist(gen);

    return files[idx].string();
}
}  // namespace dix

#endif  // DIX_RANDOM_HPP
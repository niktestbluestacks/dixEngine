#ifndef DIX_RANDOM_HPP
#define DIX_RANDOM_HPP

// dix
#include <Utils/Converter.hpp>

// std
#include <cstdlib>
#include <filesystem>
#include <vector>
#ifndef __clang__
#include <random>
#endif // __clang__

namespace dix {
namespace fs = std::filesystem;
inline std::string getRandomFile(std::string filepath = toAudioPath("")) {
    if (filepath.back() == '/') filepath.pop_back();

    std::vector<fs::path> files(0);
    for (auto const& entry : fs::directory_iterator(filepath)) {
        if (fs::is_regular_file(entry.status())) files.push_back(entry.path());
    }

    std::size_t idx = static_cast<std::size_t>(std::rand()) % files.size();

    return files[idx].string();
}
}  // namespace dix

#endif  // DIX_RANDOM_HPP
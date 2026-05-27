#ifndef CONVERTER_HPP
#define CONVERTER_HPP

// std
#include <string>

namespace dix {
inline constexpr std::string toShaderPath(const std::string& shaderName) {
    return std::string(static_cast<std::string>("../dixEngine/build/shaders/") +
                       shaderName);
}

inline constexpr std::string toModelPath(const std::string& modelFilepath) {
    return std::string(
        static_cast<std::string>("../dixEngine/Applications/models/") +
        modelFilepath);
}

inline constexpr std::string toAudioPath(const std::string& audioFilepath) {
    return std::string(
        static_cast<std::string>("../dixEngine/Applications/audio/") +
        audioFilepath);
}

inline constexpr std::string toRecordingPath(
    const std::string& recordingFilepath) {
    return std::string(static_cast<std::string>("../dixEngine/recording/") +
                       recordingFilepath);
}
}  // namespace dix

#endif  // CONVERTER_HPP
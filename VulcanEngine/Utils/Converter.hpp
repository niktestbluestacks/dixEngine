#ifndef CONVERTER_HPP
#define CONVERTER_HPP

// std
#include <string>

namespace dix {
inline constexpr std::string dixToShaderPath(const std::string& shaderName) {
    return std::string(static_cast<std::string>("../dixEngine/build/shaders/") +
                       shaderName);
}

inline constexpr std::string dixToModelPath(const std::string& modelFilepath) {
    return std::string(
        static_cast<std::string>("../dixEngine/Applications/models/") +
        modelFilepath);
}

inline constexpr std::string dixToAudioPath(const std::string& audioFilepath) {
    return std::string(
        static_cast<std::string>("../dixEngine/Applications/audio/") +
        audioFilepath);
}

inline constexpr std::string dixToRecordingPath(
    const std::string& recordingFilepath) {
    return std::string(static_cast<std::string>("../dixEngine/recording/") +
                       recordingFilepath);
}
}  // namespace dix

#define toShaderPath(filepath) ::dix::dixToShaderPath(filepath)
#define toModelPath(filepath) ::dix::dixToModelPath(filepath)
#define toAudioPath(filepath) ::dix::dixToAudioPath(filepath)
#define toRecordingPath(filepath) ::dix::dixToRecordingPath(filepath)
#endif  // CONVERTER_HPP
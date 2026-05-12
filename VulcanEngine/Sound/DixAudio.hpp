#ifndef DIX_AUDIO_HPP
#define DIX_AUDIO_HPP

// dix
#include <Utils/Class.hpp>

// std
#include <string>
#include <memory>

// libs
#include <glm/glm.hpp>

namespace dix {
class DixAudio {
public:
    DixAudio();
    DixAudio(const std::string& filepath);
    ~DixAudio();

    DIX_DISABLE_COPY(DixAudio)

    DixAudio(DixAudio&& other) noexcept;
    DixAudio& operator=(DixAudio&& other) noexcept;

    bool loadFromFile(const std::string& filepath);

    void play(bool loop = false);
    void stop();
    void pause();
    void resume();

    void setVolume(float volume);
    float getVolume() const;

    void setPan(float pan);
    float getPan() const;

    void seekTo(float seconds);

    /** 
    @brief Position in audio in seconds
    @return Position in audio (second) 
    **/
    float getPosition() const;
    float getDuration() const;

    // State queries
    bool isPlaying() const;
    bool isPaused() const;
    bool isLoaded() const;

    void setPosition(const glm::vec3& pos);
    void setMinDistance(float dist); // Distance where attenuation starts
    void setMaxDistance(float dist);

    void updateListener(const glm::vec3& position, const glm::vec3& forward, const glm::vec3& up);
private:
    class Impl;
    std::unique_ptr <Impl> m_Impl;

};
}   // namespace dix

#endif // DIX_AUDIO_HPP
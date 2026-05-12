// dix
#include <Sound/DixAudio.hpp>
#include <Logger/Logger.hpp>

// libs
#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio.h>

// std
#include <cassert>

namespace dix {
class DixAudio::Impl {
public:
    ma_engine engine;
    ma_sound* sound;
    float volume;
    float pan;
    bool isSoundLoaded;

    Impl() : sound { nullptr },
    volume { 1.f },
    pan { 0.f },
    isSoundLoaded { false } {
        ma_result result = ma_engine_init(nullptr, &engine);
        if (result != MA_SUCCESS) {
            DixLogErr("Failer to initialize multiaudio engine, result = {}", static_cast <int> (result));
        }
    }

    ~Impl() {
        if (sound) {
            ma_sound_uninit(sound);
            delete sound;
        }
        ma_engine_uninit(&engine);
    }

    bool loadFromFile(const std::string& filepath) {
        if (sound) {
            ma_sound_uninit(sound);
            delete sound;
            sound = nullptr;
            isSoundLoaded = false;
        }

        assert(!sound && "Something went wrong when loading from file...");
        sound = new ma_sound;

        ma_result result = ma_sound_init_from_file(
            &engine, 
            filepath.c_str(), 
            0, nullptr, 
            nullptr, 
            sound
        );

        if (result != MA_SUCCESS) {
            DixLogErr("failed to load the sound from file: {},\n Error: {}", filepath, static_cast <int> (result));
            delete sound;
            sound = nullptr;
            return false;
        }

        isSoundLoaded = true;

        ma_sound_set_volume(sound, volume);
        updatePan();
        return true;
    }

    void play(bool loopFlag) {
        if (!sound) {
            DixLogWarn("No sound was provided but tried to play");
            return;
        }

        ma_sound_set_looping(sound, loopFlag ? MA_TRUE : MA_FALSE);
        ma_sound_start(sound);
    }

    void stop() {
        if (!sound) {
            DixLogWarn("Tried to stop sound when it was not existing");
            return;
        }

        ma_sound_stop(sound);
        ma_sound_seek_to_pcm_frame(sound, 0);
    }

    void pause() {
        if (!sound) {
            DixLogWarn("Tried to pause sound when it is not existing");
            return;
        }
        ma_sound_stop(sound);
    }

    void resume() {
        if (!sound) {
            DixLogWarn("Tried to play sound when it is not existing");
            return;
        }
        ma_sound_start(sound);
    }

    void setVolume(float vol) {
        volume = std::max(0.f, std::min(1.f, vol));
        if (sound) {
            ma_sound_set_volume(sound, volume);
        }
    }

    float getVolume() const {
        return volume;
    }

    void setPan(float p) {
        pan = std::max(-1.f, std::min(1.f, p));
        updatePan();
    }

    float getPan() const { 
        return pan; 
    }

    void seekTo(float seconds) {
        if (!sound) return;
        ma_uint64 sampleRate = ma_engine_get_sample_rate(&engine);
        if (sampleRate == 0) return;
        ma_uint64 frameIndex = static_cast<ma_uint64>(seconds * sampleRate);
        ma_sound_seek_to_pcm_frame(sound, frameIndex);
    }

    float getPosition() const {
        if (!sound) return 0.0f;
        ma_uint64 frame;
        ma_sound_get_length_in_pcm_frames(sound, &frame);
        ma_uint64 sampleRate = ma_engine_get_sample_rate(&engine);
        if (sampleRate == 0) return 0.0f;
        return static_cast<float>(frame) / static_cast<float>(sampleRate);
    }

    float getDuration() const {
        if (!sound) return 0.0f;
        ma_uint64 frame;
        ma_uint64 totalFrames = ma_sound_get_length_in_pcm_frames(sound, &frame);
        ma_uint64 sampleRate = ma_engine_get_sample_rate(&engine);
        if (sampleRate == 0) return 0.0f;
        return static_cast<float>(totalFrames) / static_cast<float>(sampleRate);
    }

    bool isPlaying() const {
        return sound ? (ma_sound_is_playing(sound) == MA_TRUE) : false;
    }

    bool isPaused() const {
        if (!sound) return false;
        return !ma_sound_is_playing(sound) && !ma_sound_at_end(sound);
    }

    bool isLoaded() const { 
        return isSoundLoaded; 
    }

private:
    void updatePan() {
        if (!sound) return;
        // miniaudio handles stereo panning internally with ma_sound_set_pan
        // Range is -1.0 (left) to 1.0 (right)
        ma_sound_set_pan(sound, pan);
    }
};

DixAudio::DixAudio() : m_Impl(std::make_unique<Impl>()) {}

DixAudio::DixAudio(const std::string& filepath): m_Impl(std::make_unique<Impl>()) {
    m_Impl->loadFromFile(filepath);
}

DixAudio::~DixAudio() = default;

DixAudio::DixAudio(DixAudio&& other) noexcept = default;

DixAudio& DixAudio::operator=(DixAudio&& other) noexcept = default;

bool DixAudio::loadFromFile(const std::string& filepath) {
    return m_Impl->loadFromFile(filepath);
}

void DixAudio::play(bool loop) {
    m_Impl->play(loop);
}

void DixAudio::stop() {
    m_Impl->stop();
}

void DixAudio::pause() {
    m_Impl->pause();
}

void DixAudio::resume() {
    m_Impl->resume();
}

void DixAudio::setVolume(float volume) {
    m_Impl->setVolume(volume);
}

float DixAudio::getVolume() const {
    return m_Impl->getVolume();
}

void DixAudio::setPan(float pan) {
    m_Impl->setPan(pan);
}

float DixAudio::getPan() const {
    return m_Impl->getPan();
}

void DixAudio::seekTo(float seconds) {
    m_Impl->seekTo(seconds);
}

float DixAudio::getPosition() const {
    return m_Impl->getPosition();
}

float DixAudio::getDuration() const {
    return m_Impl->getDuration();
}

bool DixAudio::isPlaying() const {
    return m_Impl->isPlaying();
}

bool DixAudio::isPaused() const {
    return m_Impl->isPaused();
}

bool DixAudio::isLoaded() const {
    return m_Impl->isLoaded();
}

}   // namespace dix
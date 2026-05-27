#ifndef DIX_AUDIO_OBJ
#define DIX_AUDIO_OBJ

// dix
#include <Model/GameObject/GameObject.hpp>
#include <Sound/DixAudio.hpp>

namespace dix {
class DixAudioObj {
   public:
    DIX_DISABLE_COPY(DixAudioObj)
    DIX_ENABLE_MOVE(DixAudioObj)
    ~DixAudioObj() = default;

    GameObject gameObject;
    DixAudio dixAudio;
};
}  // namespace dix

#endif  // DIX_AUDIO_OBJ
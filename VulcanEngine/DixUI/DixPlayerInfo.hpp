#ifndef DIX_PLAYER_INFO_HPP
#define DIX_PLAYER_INFO_HPP

// dix
#include <Input/Keyboard/KeyboardAndMouseController.hpp>
#include <UI/DixUIElement.hpp>


// libs
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

namespace dix {
class DixPlayerInfo : public DixUIElement {
   public:
    using DixUIElement::DixUIElement;
    DixPlayerInfo(const DixUIInfo& info, glm::vec3 playerPosition)
        : DixUIElement(info), m_playerPosition(playerPosition) {}

    void update(float dt, const AdditionalUIInfo& additionalInfo) override;

   private:
    glm::vec3 m_playerPosition;
};
}  // namespace dix

#endif  // DIX_PLAYER_INFO_HPP
// dix
#include <DixUI/DixPlayerInfo.hpp>

namespace dix {
void DixPlayerInfo::update(float dt, const AdditionalUIInfo& additionalInfo) {
    m_playerPosition = additionalInfo.playerPosition;
    std::string text = "Player Pos: X:" + 
                        std::to_string(m_playerPosition.x) + " Y:" + 
                        std::to_string(m_playerPosition.y) + " Z:" + 
                        std::to_string(m_playerPosition.z);
    buildVerticesForText(text, 8.f, 52.f);

}
}   // namespace dix
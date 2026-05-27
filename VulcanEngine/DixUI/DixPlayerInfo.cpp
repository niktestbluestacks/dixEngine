// dix
#include <DixUI/DixPlayerInfo.hpp>

// std
#include <cstdio>

namespace dix {
void DixPlayerInfo::update(float dt, const AdditionalUIInfo& additionalInfo) {
    m_playerPosition = additionalInfo.playerPosition;
    char bufferx[50];
    char buffery[50];
    char bufferz[50];
    std::sprintf(bufferx, "%.3f", m_playerPosition.x);
    std::sprintf(buffery, "%.3f", m_playerPosition.y);
    std::sprintf(bufferz, "%.3f", m_playerPosition.z);
    std::string text = "Player Pos: X:" + std::string(bufferx) +
                       " Y:" + std::string(buffery) +
                       " Z:" + std::string(bufferz);
    buildVerticesForText(text, 8.f, 52.f);
}
}  // namespace dix
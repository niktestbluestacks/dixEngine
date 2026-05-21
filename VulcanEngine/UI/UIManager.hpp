#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <UI/DixUIElement.hpp>
#include <vector>
#include <memory>

namespace dix {
class UIManager {
public:
    void addElement(std::unique_ptr<DixUIElement> elem) { m_elements.push_back(std::move(elem)); }
    void update(float dt, const AdditionalUIInfo& additionalInfo) { for (auto &e : m_elements) e->update(dt, additionalInfo); }
    void render(FrameInfo& fi) { for (auto &e : m_elements) e->render(fi); }
    void upload(FrameInfo& fi) { for (auto &e : m_elements) e->upload(fi); }
private:
    std::vector<std::unique_ptr<DixUIElement>> m_elements;
};
}   // namespace dix

#endif // UI_MANAGER_HPP
#pragma once

#include <UI/IUIElement.hpp>
#include <vector>
#include <memory>

namespace dix {
class UIManager {
public:
    void addElement(std::unique_ptr<IUIElement> elem) { m_elements.push_back(std::move(elem)); }
    void update(float dt) { for (auto &e : m_elements) e->update(dt); }
    void render(FrameInfo& fi) { for (auto &e : m_elements) e->render(fi); }
    void upload(FrameInfo& fi) { for (auto &e : m_elements) e->upload(fi); }
private:
    std::vector<std::unique_ptr<IUIElement>> m_elements;
};
}

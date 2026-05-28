#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

// dix
#include <UI/DixUIElement.hpp>

// std
#include <memory>
#include <thread>
#include <vector>


namespace dix {
class UIManager {
   public:
    void addElement(std::unique_ptr<DixUIElement> elem) {
        m_elements.push_back(std::move(elem));
    }
    void update(float dt, const AdditionalUIInfo& additionalInfo) {
        // std::vector<std::jthread> updates;
        // updates.reserve(m_elements.size());

        // for (size_t thread = 0; thread < m_elements.size(); ++thread) {
        //     updates.emplace_back(&DixUIElement::update, m_elements[thread].get(), dt, additionalInfo);
        // }
        for (auto& e : m_elements) e->update(dt, additionalInfo);
    }
    void render(FrameInfo& fi) {
        for (auto& e : m_elements) e->render(fi);
    }
    void upload(FrameInfo& fi) {
        for (auto& e : m_elements) e->upload(fi);
    }

   private:
    std::vector<std::unique_ptr<DixUIElement>> m_elements;
};
}  // namespace dix

#endif  // UI_MANAGER_HPP
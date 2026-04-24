#pragma once

#include <UI/IUIElement.hpp>
#include <UI/UIRenderer.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>

#include <string>
#include <vector>
#include <unordered_map>

namespace dix {

struct GlyphInfo { float u0, u1; int px; };

class FpsCounter : public IUIElement {
public:
    FpsCounter(UIRenderer& uiRenderer, VkExtent2D screenExtent, const std::string& fontTxtPath, const std::string& fontTgaPath);
    ~FpsCounter();

    void update(float dt) override;
    void render(FrameInfo& fi) override;
    void upload(FrameInfo& fi) override;

private:
    void loadFontTxt(const std::string& path);
    void loadFontAtlas(const std::string& path);
    void buildVerticesForText(const std::string& text);

private:
    UIRenderer& m_uiRenderer;
    VkExtent2D m_screenExtent;
    UITexture m_fontTexture;
    std::unordered_map<char, GlyphInfo> m_glyphs;

    std::vector<std::unique_ptr<DixBuffer>> m_vertexBuffers;
    uint32_t m_vertexCount = 0;
    uint32_t m_vertexCapacity = 0;
    std::vector<char> m_vertexStaging; // CPU-side copy of vertex data

    // font atlas pixels
    std::vector<unsigned char> m_fontPixels;
    int m_fontWidth = 0;
    int m_fontHeight = 0;

    float m_acc = 0.f;
    int m_frames = 0;
    int m_fps = 0;
    std::string m_lastText;
};

}

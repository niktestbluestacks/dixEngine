#ifndef DIX_UI_ELEMENT_HPP
#define DIX_UI_ELEMENT_HPP

// dix
#include <UI/UIRenderer.hpp>
#include <Utils/FrameInfo.hpp>

// std
#include <string>

namespace dix {
struct DixUIVert {
    float x, y;  // position in pixels relative to top-left of screen
    float u, v;  // uv coordinates in font atlas
};

struct DixGlyphInfo {
    float u0, u1;
    int px;
};

struct AdditionalUIInfo {
    glm::vec3 playerPosition;
    vk::Extent2D screenExtent;
};

struct DixUIInfo {
    UIRenderer& uiRenderer;
    vk::Extent2D screenExtent;
    std::string name = "";
    // DixUIVert vert;
    std::string fontTxtPath = "UI/font.txt";
    std::string fontTgaPath = "UI/font02.tga";
    int fontHeight = 3;
};

class DixUIElement {
   public:
    DixUIElement(const DixUIInfo& info);
    DixUIElement(DixUIElement&&) = default;
    virtual ~DixUIElement();

   protected:
    virtual void loadFontTxt(const std::string& path);
    virtual void loadFontAtlas(const std::string& path);

   public:
    virtual void update(float dt, const AdditionalUIInfo& additionalInfo);
    // update CPU-side state
    virtual void render(FrameInfo& fi);
    // upload GPU resources for the upcoming frame (called after beginFrame)
    virtual void upload(FrameInfo& fi);

   protected:
    void buildVerticesForText(const std::string& text);
    void buildVerticesForText(const std::string& text, float x, float y);
    void clearVertices() { m_vertexCount = 0; m_vertexStaging.clear(); }

   protected:
    UIRenderer& m_uiRenderer;
    vk::Extent2D m_screenExtent;
    UITexture m_fontTexture;
    std::unordered_map<char, DixGlyphInfo> m_glyphs;

    std::vector<std::unique_ptr<DixBuffer>> m_vertexBuffers;
    uint32_t m_vertexCount = 0;
    uint32_t m_vertexCapacity = 0;
    std::vector<char> m_vertexStaging;  // CPU-side copy of vertex data

    // font atlas pixels
    std::vector<unsigned char> m_fontPixels;

    int m_fontWidth = 0;
    int m_fontHeight = 3;
};
}  // namespace dix

#endif  // DIX_UI_ELEMENT_HPP
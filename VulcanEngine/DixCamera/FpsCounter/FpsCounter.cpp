#include "FpsCounter.hpp"
#include <Utils/Converter.hpp>

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace dix {

struct UiVert { float x,y,u,v; };

FpsCounter::FpsCounter(UIRenderer& uiRenderer, VkExtent2D screenExtent, const std::string& fontTxtPath, const std::string& fontTgaPath) :
    m_uiRenderer(uiRenderer), m_screenExtent(screenExtent) {
    loadFontTxt(fontTxtPath);
    loadFontAtlas(fontTgaPath);
    // create texture in ui renderer
    m_fontTexture = m_uiRenderer.createTextureFromPixels(m_fontPixels.data(), m_fontWidth, m_fontHeight);
}


// No i am not

FpsCounter::~FpsCounter() {
    // texture cleanup left to UIRenderer (currently nothing)
}

void FpsCounter::loadFontTxt(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) throw std::runtime_error("failed to open font txt");
    std::string line;
    while (std::getline(in,line)) {
        std::istringstream ss(line);
        int code; std::string sym; float u0,u1; int px;
        ss >> code >> sym >> u0 >> u1 >> px;
        char c = sym[0];
        m_glyphs[c] = GlyphInfo{u0,u1,px};
    }
}

void FpsCounter::loadFontAtlas(const std::string& path) {
    // Minimal uncompressed TGA loader (assumes type 2, true-color)
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) throw std::runtime_error("failed to open font tga");
    unsigned char header[18];
    in.read(reinterpret_cast<char*>(header), 18);
    if (!in) throw std::runtime_error("failed to read tga header");
    unsigned short width = header[12] | (header[13] << 8);
    unsigned short height = header[14] | (header[15] << 8);
    unsigned char depth = header[16];
    unsigned char type = header[2];
    if (type != 2) throw std::runtime_error("unsupported tga type");
    int channels = depth / 8;
    size_t rawSize = static_cast<size_t>(width) * height * channels;
    std::vector<unsigned char> raw(rawSize);
    in.read(reinterpret_cast<char*>(raw.data()), rawSize);
    if (!in) throw std::runtime_error("failed to read tga image data");

    // convert to RGBA
    m_fontWidth = width; m_fontHeight = height;
    m_fontPixels.resize(static_cast<size_t>(width) * height * 4);
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
        unsigned char b = raw[i * channels + 0];
        unsigned char g = raw[i * channels + 1];
        unsigned char r = raw[i * channels + 2];
        unsigned char a = (channels >= 4) ? raw[i * channels + 3] : 255;
        m_fontPixels[i * 4 + 0] = r;
        m_fontPixels[i * 4 + 1] = g;
        m_fontPixels[i * 4 + 2] = b;
        m_fontPixels[i * 4 + 3] = a;
    }
}

void FpsCounter::update(float dt) {
    m_acc += dt; ++m_frames;
    if (m_acc >= 1.0f) {
        m_fps = static_cast<int>(std::round((float)m_frames / m_acc));
        m_frames = 0; m_acc = 0.f;
    }
}

void FpsCounter::buildVerticesForText(const std::string& text) {
    std::vector<UiVert> verts;
    float x = 8.f;
    float y = 8.f;
    for (char c : text) {
        auto it = m_glyphs.find(c);
        if (it==m_glyphs.end()) continue;
        GlyphInfo g = it->second;
        float gw = static_cast<float>(g.px);
        float gh = static_cast<float>(m_fontHeight);
        float u0 = g.u0;
        float u1 = g.u1;
        float v0 = 0.f, v1 = 1.f;
        // two tris
        verts.push_back({x,y,u0,v0});
        verts.push_back({x+gw,y,u1,v0});
        verts.push_back({x+gw,y+gh,u1,v1});
        verts.push_back({x,y,u0,v0});
        verts.push_back({x+gw,y+gh,u1,v1});
        verts.push_back({x,y+gh,u0,v1});
        x += gw + 1.f;
    }

    if (verts.empty()) return;
    m_vertexCount = static_cast<uint32_t>(verts.size());
    m_vertexBuffer = std::make_unique<DixBuffer>(m_uiRenderer.getDevice(), sizeof(UiVert), m_vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    DixBuffer staging(m_uiRenderer.getDevice(), sizeof(UiVert), m_vertexCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging.map();
    staging.writeToBuffer(verts.data(), sizeof(UiVert)*verts.size(), 0);
    staging.flush();
    m_uiRenderer.getDevice().copyBuffer(staging.getBuffer(), m_vertexBuffer->getBuffer(), sizeof(UiVert)*verts.size());
}

void FpsCounter::render(FrameInfo& fi) {
    std::string text = std::to_string(m_fps) + " FPS";
    if (text!=m_lastText) {
        buildVerticesForText(text);
        m_lastText = text;
    }
    if (!m_vertexBuffer) return;
    // bind descriptor set for font
    VkPipelineLayout layout = m_uiRenderer.getPipelineLayout();
    vkCmdBindDescriptorSets(fi.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &m_fontTexture.descriptorSet, 0, nullptr);

    VkBuffer b = m_vertexBuffer->getBuffer();
    VkDeviceSize off = 0;
    vkCmdBindVertexBuffers(fi.commandBuffer, 0, 1, &b, &off);
    // set viewport to screen size so vertex positions in pixels map correctly
    VkViewport vp{};
    vp.x = 0.0f; vp.y = 0.0f; vp.width = static_cast<float>(m_screenExtent.width); vp.height = static_cast<float>(m_screenExtent.height); vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
    vkCmdSetViewport(fi.commandBuffer, 0, 1, &vp);
    VkRect2D scissor{{0,0}, m_screenExtent};
    vkCmdSetScissor(fi.commandBuffer, 0, 1, &scissor);
    vkCmdDraw(fi.commandBuffer, m_vertexCount, 1, 0, 0);
}

}
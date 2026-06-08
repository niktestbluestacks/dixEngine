// dix
#include <UI/DixUIElement.hpp>
#include <Utils/Converter.hpp>

// std
#include <fstream>
#include <sstream>

namespace dix {

DixUIElement::DixUIElement(const DixUIInfo& info)
    : m_uiRenderer(info.uiRenderer),
      m_screenExtent(info.screenExtent),
      m_fontHeight(info.fontHeight) {
    loadFontTxt(info.fontTxtPath);
    loadFontAtlas(info.fontTgaPath);

    m_fontTexture = m_uiRenderer.createTextureFromPixels(
        m_fontPixels.data(), m_fontWidth, m_fontHeight);
    // preallocate a reasonably large host-visible vertex buffer to avoid
    // reallocations during runtime (which can collide with GPU usage and cause
    // submit failures).
    const uint32_t initialCapacity = 2048;  // number of vertices
    m_vertexCapacity = initialCapacity;
    // create one host-visible buffer per frame in flight to avoid CPU/GPU races
    const size_t kFramesInFlight =
        2;  // match renderer swapchain frames in flight
    m_vertexBuffers.resize(kFramesInFlight);
    for (size_t i = 0; i < m_vertexBuffers.size(); ++i) {
        m_vertexBuffers[i] = std::make_unique<DixBuffer>(
            m_uiRenderer.getDevice(), sizeof(DixUIVert), m_vertexCapacity,
            vk::BufferUsageFlagBits::eVertexBuffer,
            vk::MemoryPropertyFlags(vk::MemoryPropertyFlagBits::eHostVisible) |
                vk::MemoryPropertyFlags(
                    vk::MemoryPropertyFlagBits::eHostCoherent));
        m_vertexBuffers[i]->map();
    }
    m_vertexStaging.reserve(initialCapacity * sizeof(DixUIVert));
}

DixUIElement::~DixUIElement() = default;

void DixUIElement::loadFontTxt(const std::string& path) {
    std::ifstream in(toModelPath(path));
    if (!in.is_open()) throw std::runtime_error("failed to open font txt");
    std::string line;
    while (std::getline(in, line)) {
        std::istringstream ss(line);
        int code;
        std::string sym;
        float u0, u1;
        int px;
        ss >> code >> sym >> u0 >> u1 >> px;
        if (sym == "space") sym = " ";
        char c = sym[0];
        m_glyphs[c] = DixGlyphInfo{u0, u1, px};
    }
}

void DixUIElement::loadFontAtlas(const std::string& path) {
    // Minimal uncompressed TGA loader (assumes type 2, true-color)
    std::ifstream in(toModelPath(path), std::ios::binary);
    if (!in.is_open()) throw std::runtime_error("failed to open font tga");
    unsigned char header[18];
    in.read(reinterpret_cast<char*>(header), 18);
    if (!in) throw std::runtime_error("failed to read tga header");
    uint16_t width = header[12] | (header[13] << 8);
    uint16_t height = header[14] | (header[15] << 8);
    unsigned char depth = header[16];
    unsigned char type = header[2];
    if (type != 2) throw std::runtime_error("unsupported tga type");
    int channels = depth / 8;
    size_t rawSize = static_cast<size_t>(width) * height * channels;
    std::vector<unsigned char> raw(rawSize);
    in.read(reinterpret_cast<char*>(raw.data()), rawSize);
    if (!in) throw std::runtime_error("failed to read tga image data");

    // convert to RGBA
    m_fontWidth = width;
    m_fontHeight = height;
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

void DixUIElement::update(float dt, const AdditionalUIInfo& additionalInfo) {
    // default implementation does nothing
}

void DixUIElement::upload(FrameInfo& fi) {
    // copy CPU staging into the per-frame mapped buffer for this frame index
    if (m_vertexStaging.empty()) return;
    int idx = fi.frameIndex % static_cast<int>(m_vertexBuffers.size());
    auto& buf = m_vertexBuffers[idx];
    if (!buf) return;
    buf->writeToBuffer(m_vertexStaging.data(), m_vertexStaging.size(), 0);
    buf->flush();
}

void DixUIElement::render(FrameInfo& fi) {
    if (m_vertexBuffers.empty()) return;
    // bind descriptor set for font
    vk::PipelineLayout layout = m_uiRenderer.getPipelineLayout();
    fi.commandBuffer.bindDescriptorSets(
        vk::PipelineBindPoint::eGraphics, layout, 0, 1,
        &m_fontTexture.descriptorSet, 0, nullptr);

    int idx = fi.frameIndex % static_cast<int>(m_vertexBuffers.size());
    vk::Buffer b = m_vertexBuffers[idx]->getBuffer();
    vk::DeviceSize off = 0;
    fi.commandBuffer.bindVertexBuffers(0, 1, &b, &off);

    // set viewport to screen size so vertex positions in pixels map correctly
    vk::Viewport vp{};
    vp.x = 0.0f;
    vp.y = 0.0f;
    vp.width = static_cast<float>(fi.screenExtent.width);
    vp.height = static_cast<float>(fi.screenExtent.height);
    vp.minDepth = 0.0f;
    vp.maxDepth = 1.0f;

    fi.commandBuffer.setViewport(0, 1, &vp);
    vk::Rect2D scissor{{0, 0}, fi.screenExtent};
    fi.commandBuffer.setScissor(0, 1, &scissor);
    fi.commandBuffer.draw(m_vertexCount, 1, 0, 0);
}

void DixUIElement::buildVerticesForText(const std::string& text, const glm::vec4& color) {
    std::vector<DixUIVert> verts;
    float x = 8.f;
    float y = 8.f;
    float startX = x;
    for (char c : text) {
        if (c == '\n') {
            x = startX;
            y -= m_fontHeight;
            continue;
        }
        auto it = m_glyphs.find(c);
        if (it == m_glyphs.end()) continue;
        DixGlyphInfo g = it->second;
        float gw = static_cast<float>(g.px);
        float gh = static_cast<float>(m_fontHeight);
        float u0 = g.u0;
        float u1 = g.u1;
        float v0 = 0.f, v1 = 1.f;
        // two tris
        verts.push_back({x, y, u0, v0, color});
        verts.push_back({x + gw, y, u1, v0, color});
        verts.push_back({x + gw, y + gh, u1, v1, color});
        verts.push_back({x, y, u0, v0, color});
        verts.push_back({x + gw, y + gh, u1, v1, color});
        verts.push_back({x, y + gh, u0, v1, color});
        x += gw + 1.f;
    }

    if (verts.empty()) return;
    m_vertexCount = static_cast<uint32_t>(verts.size());
    // serialize CPU copy into staging vector
    m_vertexStaging.resize(sizeof(DixUIVert) * verts.size());
    memcpy(m_vertexStaging.data(), verts.data(), m_vertexStaging.size());
    if (m_vertexCapacity < m_vertexCount) {
        // grow existing per-frame buffers
        m_vertexCapacity = m_vertexCount;
        m_uiRenderer.getDevice().device().waitIdle();
        for (size_t i = 0; i < m_vertexBuffers.size(); ++i) {
            m_vertexBuffers[i] = std::make_unique<DixBuffer>(
                m_uiRenderer.getDevice(), sizeof(DixUIVert), m_vertexCapacity,
                vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlags(
                    vk::MemoryPropertyFlagBits::eHostVisible) |
                    vk::MemoryPropertyFlags(
                        vk::MemoryPropertyFlagBits::eHostCoherent));
            m_vertexBuffers[i]->map();
        }
    }
}

void DixUIElement::buildVerticesForText(const std::string& text, float x,
                                        float y, const glm::vec4& color) {
    std::vector<DixUIVert> verts;
    float startX = x;
    for (char c : text) {
        if (c == '\n') {
            x = startX;
            y -= m_fontHeight;
            continue;
        }
        auto it = m_glyphs.find(c);
        if (it == m_glyphs.end()) continue;
        DixGlyphInfo g = it->second;
        float gw = static_cast<float>(g.px);
        float gh = static_cast<float>(m_fontHeight);
        float u0 = g.u0;
        float u1 = g.u1;
        float v0 = 0.f, v1 = 1.f;
        // two tris
        verts.push_back({x, y, u0, v0, color});
        verts.push_back({x + gw, y, u1, v0, color});
        verts.push_back({x + gw, y + gh, u1, v1, color});
        verts.push_back({x, y, u0, v0, color});
        verts.push_back({x + gw, y + gh, u1, v1, color});
        verts.push_back({x, y + gh, u0, v1, color});
        x += gw + 1.f;
    }

    if (verts.empty()) return;
    m_vertexCount = static_cast<uint32_t>(verts.size());
    // serialize CPU copy into staging vector
    m_vertexStaging.resize(sizeof(DixUIVert) * verts.size());
    memcpy(m_vertexStaging.data(), verts.data(), m_vertexStaging.size());
    if (m_vertexCapacity < m_vertexCount) {
        // grow existing per-frame buffers
        m_vertexCapacity = m_vertexCount;
        m_uiRenderer.getDevice().device().waitIdle();
        for (size_t i = 0; i < m_vertexBuffers.size(); ++i) {
            m_vertexBuffers[i] = std::make_unique<DixBuffer>(
                m_uiRenderer.getDevice(), sizeof(DixUIVert), m_vertexCapacity,
                vk::BufferUsageFlagBits::eVertexBuffer,
                vk::MemoryPropertyFlags(
                    vk::MemoryPropertyFlagBits::eHostVisible) |
                    vk::MemoryPropertyFlags(
                        vk::MemoryPropertyFlagBits::eHostCoherent));
            m_vertexBuffers[i]->map();
        }
    }
}

}  // namespace dix
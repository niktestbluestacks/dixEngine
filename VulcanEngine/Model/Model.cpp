// dix
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Model/Model.hpp>
#include <Logger/Logger.hpp>
#include <Utils/Hash.hpp>
#include <Model/DixTexture/DixTexture.hpp>

// libs
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

// std
#include <cassert>
#include <cstring>
#include <unordered_map>
#include <fstream>
#include <filesystem>

namespace std {
template <>
struct hash <dix::Model::Vertex> {
	size_t operator()(dix::Model::Vertex const& vertex) const {
		size_t seed = 0;
		dix::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
		return seed;
	}
};

}	// namespace std

namespace dix {
Model::Model(EngineDevice& dixDevice, const Model::Builder& builder) :
		m_dixDevice{ dixDevice } {
	createVertexBuffers(builder.vertices);
	createIndexBuffers(builder.indices);

	// copy texture handles from builder into the model so renderer can access them
	if (builder.texture.getImageView() != VK_NULL_HANDLE && builder.texture.getSampler() != VK_NULL_HANDLE) {
		m_textureInfo.view = builder.texture.getImageView();
		m_textureInfo.sampler = builder.texture.getSampler();
	}
}

Model::~Model() {}

void Model::createVertexBuffers(const std::vector<Vertex>& vertices) {
	vertexCount = static_cast <uint32_t> (vertices.size());
	assert(vertexCount >= 3 && "Vertex count must be at least 3");
	uint32_t vertexSize = sizeof(vertices[0]);
	VkDeviceSize bufferSize = vertexSize * vertexCount;

	DixBuffer stagingBuffer {
		m_dixDevice,
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	// can't static cast vecause MSVC are p****s
	stagingBuffer.writeToBuffer((void*) vertices.data());

	m_vertexBuffer = std::make_unique <DixBuffer> (
		m_dixDevice,
		vertexSize,
		vertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	m_dixDevice.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
}

void Model::createIndexBuffers(const std::vector<uint32_t>& indices) {
	indexCount = static_cast <uint32_t> (indices.size());
	m_hasIndexBuffer = indexCount > 0;
	if (!m_hasIndexBuffer) {
		return;
	}
	uint32_t indexSize = sizeof(indices[0]);
	VkDeviceSize bufferSize = indexSize * indexCount;

	DixBuffer stagingBuffer{
		m_dixDevice,
		indexSize,
		indexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	stagingBuffer.map();
	// can't static cast vecause MSVC are p****s
	stagingBuffer.writeToBuffer((void*) indices.data());

	m_indexBuffer = std::make_unique <DixBuffer> (
		m_dixDevice,
		indexSize,
		indexCount,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);

	m_dixDevice.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);

}

void Model::draw(VkCommandBuffer commandBuffer) {
	if (m_hasIndexBuffer) {
		vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
	}
	else {
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}
}

std::unique_ptr<Model> Model::createModelFromFile(
		EngineDevice& dixDevice, 
		const std::string& filepath) {
	Builder builder{};
	builder.loadModel(filepath, dixDevice);

	return std::make_unique <Model>(dixDevice, builder);
}

void Model::bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = { m_vertexBuffer->getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (m_hasIndexBuffer) {
		vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescriptions() {
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescriptions;
}
std::vector<VkVertexInputAttributeDescription> Model::Vertex::getAttributeDescriptions() {
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.reserve(4);

	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) });
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) });
	attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv) });

	return attributeDescriptions;
}

void Model::Builder::loadModel(const std::string& filepath, EngineDevice& device) {
    tinyobj::attrib_t attrib{};
    std::vector<tinyobj::shape_t> shapes{};
    std::vector<tinyobj::material_t> materials{};
    std::string warn{}, err{};

    // Extract OBJ directory for MTL & texture resolution
    std::string mtlDir;
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        mtlDir = filepath.substr(0, lastSlash + 1);
    }

    // Load OBJ + MTL (mtl_basedir is a plain const char* parameter)
    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                          filepath.c_str(),
                          mtlDir.empty() ? nullptr : mtlDir.c_str())) {
        throw std::runtime_error("Failed to load model: " + warn + err);
    }
    if (!warn.empty()) {
        DixLogWarn("tinyobj warning: " + warn);
    }

    vertices.clear();
    indices.clear();
    texture = {};
    submeshes.clear();

    // Track which (shape, material) maps to which submesh index
    struct MeshKey {
        size_t shapeIdx;
        int materialId;
        bool operator==(const MeshKey& other) const {
            return shapeIdx == other.shapeIdx && materialId == other.materialId;
        }
    };
    struct MeshKeyHash {
        size_t operator()(const MeshKey& k) const {
            return std::hash<size_t>()(k.shapeIdx) ^ std::hash<int>()(k.materialId);
        }
    };

    std::unordered_map<MeshKey, size_t, MeshKeyHash> submeshMap;
    std::vector<std::unordered_map<Vertex, uint32_t>> submeshUniqueMaps; // Parallel dedup maps

    for (size_t shapeIdx = 0; shapeIdx < shapes.size(); ++shapeIdx) {
        const auto& shape = shapes[shapeIdx];
        const auto& mesh = shape.mesh;

        // Iterate over FACES, not raw indices, to correctly associate materials
        for (size_t faceIdx = 0; faceIdx < mesh.num_face_vertices.size(); ++faceIdx) {
            // Skip non-triangles
            if (mesh.num_face_vertices[faceIdx] != 3) {
                DixLogWarn("Skipping non-triangular face in shape " + std::to_string(shapeIdx));
                continue;
            }

            int matId = mesh.material_ids[faceIdx]; // -1 if no material
            MeshKey key{shapeIdx, matId};

            // Create new submesh entry if needed
            if (submeshMap.find(key) == submeshMap.end()) {
                size_t newIdx = submeshes.size();
                submeshMap[key] = newIdx;
                submeshes.emplace_back();
                submeshUniqueMaps.emplace_back(); // New dedup map for this submesh

                // Populate material info
                if (matId >= 0 && matId < static_cast<int>(materials.size())) {
                    const auto& mat = materials[matId];
                    submeshes.back().baseColor = glm::vec3(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);

                    if (!mat.diffuse_texname.empty()) {
                        std::string texName = mat.diffuse_texname;
                        bool isAbsolute = (texName.size() > 1 && texName[1] == ':') ||
                                          (!texName.empty() && (texName[0] == '/' || texName[0] == '\\'));
                        submeshes.back().texturePath = isAbsolute ? texName : (mtlDir + texName);
                    }
                }
            }

            SubMesh& currentSubmesh = submeshes[submeshMap[key]];
            auto& uniqueVertices = submeshUniqueMaps[submeshMap[key]]; // ⚠️ Per-submesh dedup

            // Process the 3 vertices of this triangle
            for (int v = 0; v < 3; ++v) {
                tinyobj::index_t idx = mesh.indices[3 * faceIdx + v];
                Vertex vertex{};

                // Position
                if (idx.vertex_index >= 0) {
                    vertex.position = glm::vec3(
                        attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2]
                    );

                    // Color: OBJ rarely has per-vertex colors. Fallback to material diffuse.
                    if (!attrib.colors.empty()) {
                        vertex.color = glm::vec3(
                            attrib.colors[3 * idx.vertex_index + 0],
                            attrib.colors[3 * idx.vertex_index + 1],
                            attrib.colors[3 * idx.vertex_index + 2]
                        );
                    } else {
                        vertex.color = currentSubmesh.baseColor;
                    }
                }

                // Normal
                if (idx.normal_index >= 0) {
                    vertex.normal = glm::vec3(
                        attrib.normals[3 * idx.normal_index + 0],
                        attrib.normals[3 * idx.normal_index + 1],
                        attrib.normals[3 * idx.normal_index + 2]
                    );
                }

                // UV Coordinates
                if (idx.texcoord_index >= 0) {
                    vertex.uv = glm::vec2(
                        attrib.texcoords[2 * idx.texcoord_index + 0],
                        1.0f - attrib.texcoords[2 * idx.texcoord_index + 1] // ✅ Flip V for Vulkan/OpenGL
                    );
                }

                // Deduplicate within this submesh
                if (uniqueVertices.find(vertex) == uniqueVertices.end()) {
                    uniqueVertices[vertex] = static_cast<uint32_t>(currentSubmesh.vertices.size());
                    currentSubmesh.vertices.push_back(vertex);
                }
                currentSubmesh.indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    // ✅ Load textures for each submesh
    for (auto& submesh : submeshes) {
        if (!submesh.texturePath.empty() && std::filesystem::exists(submesh.texturePath)) {
            DixLogDebug("Loading texture: " + submesh.texturePath);
            submesh.texture = createTextureFromFile(submesh.texturePath, device);
        } else {
            if (!submesh.texturePath.empty()) {
                DixLogWarn("Texture not found: " + submesh.texturePath + " → using fallback");
            }
            submesh.texture = createDefaultTexture(device);
        }
    }

    // 🔄 Backward compatibility: If only 1 material exists, flatten to original buffers
    if (submeshes.size() == 1) {
        vertices = std::move(submeshes[0].vertices);
        indices  = std::move(submeshes[0].indices);
        texture  = submeshes[0].texture;
    }
}

}	// namespace dix
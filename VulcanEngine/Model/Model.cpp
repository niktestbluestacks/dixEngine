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

void Model::Builder::loadModel(const std::string& filepath, EngineDevice& dixDevice) {
	tinyobj::attrib_t attrib{};
	std::vector <tinyobj::shape_t> shapes{};
	std::vector <tinyobj::material_t> materials{};
	std::string warn{};
	std::string err{};

	std::ifstream file (filepath);

	std::string line;

	bool loaded_with_texture = false;

	while (std::getline(file, line)) {
		if (line.front() == 'm') {
			std::string texture_filepath;
			texture_filepath = filepath.substr(0, filepath.find_last_of('\\') + 1);
			// texture_filepath += line.substr(line.find_last_of(' ') + 1, line.size() - 1);
			file.close();
			if (!tinyobj::LoadObj(
					&attrib, &shapes, &materials, &warn, &err, 
					filepath.c_str(), texture_filepath.c_str()
					)) {
				throw std::runtime_error(warn + err);
			}
			loaded_with_texture = true;
			break;
		}
	}
	if (!loaded_with_texture) {
		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
			throw std::runtime_error(warn + err);
		}
	}

	vertices.clear();
	indices.clear();

	std::unordered_map <Vertex, uint32_t> uniqueVertices{};
	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex{};
			if (index.vertex_index >= 0) {
				vertex.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				};

				vertex.color = {
					attrib.colors[3 * index.vertex_index + 0],
					attrib.colors[3 * index.vertex_index + 1],
					attrib.colors[3 * index.vertex_index + 2]
				};
			}

			if (index.normal_index >= 0) {
				vertex.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals[3 * index.normal_index + 2]
				};
			}

			if (index.texcoord_index >= 0) {
				vertex.uv = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					attrib.texcoords[2 * index.texcoord_index + 1],
				};
			}

			if (!uniqueVertices.contains(vertex)) {
				uniqueVertices[vertex] = static_cast <uint32_t> (vertices.size());
				vertices.push_back(vertex);
			}
			indices.push_back(uniqueVertices[vertex]);
		}
	}

    if (loaded_with_texture && !materials.empty()) {
        const tinyobj::material_t& mat = materials[0];
		if (!mat.diffuse_texname.empty()) {
			// Resolve texture path relative to the OBJ file directory unless
			// the material already supplies an absolute path.
			std::string texName = mat.diffuse_texname;
			std::string dir;
			size_t pos = filepath.find_last_of("/\\");
			if (pos != std::string::npos) dir = filepath.substr(0, pos + 1);

			bool isAbsolute = false;
			if (!texName.empty()) {
				if (texName.size() > 1 && texName[1] == ':') isAbsolute = true; // Windows drive letter
				if (!texName.empty() && (texName[0] == '/' || texName[0] == '\\')) isAbsolute = true; // Unix or root
			}

			std::string texPath = isAbsolute ? texName : (dir + texName);

			texture = createTextureFromFile(texPath, dixDevice);

		} else {
            // no texture file – use the white fallback
            texture = createDefaultTexture(dixDevice);
        }
    } else {
        // no material – use the white fallback
        texture = createDefaultTexture(dixDevice);
    }
}

}	// namespace dix
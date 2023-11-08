#include "Model.h"

#include "../utils/Log.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>
#include <set>

#include <glm/gtc/type_ptr.hpp>

namespace
{
	std::unordered_map<int, vk::Filter> filterMap;
	std::unordered_map<int, vk::SamplerAddressMode> addressMap;
}

Model::Model()
{
	filterMap[TINYGLTF_TEXTURE_FILTER_NEAREST]				  = vk::Filter::eNearest;
	filterMap[TINYGLTF_TEXTURE_FILTER_LINEAR]				  = vk::Filter::eLinear;
	filterMap[TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST] = vk::Filter::eNearest;
	filterMap[TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST]  = vk::Filter::eLinear;
	filterMap[TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR]  = vk::Filter::eNearest;
	filterMap[TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR]   = vk::Filter::eLinear;

	addressMap[TINYGLTF_TEXTURE_WRAP_REPEAT]		  = vk::SamplerAddressMode::eRepeat;
	addressMap[TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE]	  = vk::SamplerAddressMode::eClampToEdge;
	addressMap[TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT] = vk::SamplerAddressMode::eMirroredRepeat;
}

void Model::destroy()
{
	for (auto& texture : m_textures)
		texture.destroy();
}

void Model::loadFromDisk(const std::string& filename)
{
	loadGltfModel(filename);
}

void Model::loadGltfModel(const std::string& filename)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string err, warn;
	bool success = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

	if (!success)
	{
		Log::error("tinygltf error: {}", err);
		return;
	}

	if (!warn.empty())
		Log::warn("tinygltf warning: {}", warn);

	for (auto& node : model.nodes)
		loadNode(model, node);

	loadTextures(model);
	loadMaterials(model);
	Log::info("loaded gltf model: {} vertices and {} indices", m_vertexBuffer.size(), m_indexBuffer.size());
}

void Model::loadTextures(tinygltf::Model& model)
{
	std::set<int> normalTextureIndices;
	for (auto& gltfMaterial : model.materials)
	{
		int index = gltfMaterial.normalTexture.index;
		if (index == -1)
			continue;
		normalTextureIndices.insert(index);
	}

	for (auto& gltfSampler : model.samplers)
	{
		std::shared_ptr<Sampler> sampler = std::make_shared<Sampler>();
		sampler->create(filterMap[gltfSampler.magFilter], addressMap[gltfSampler.wrapS]);
		m_samplers.emplace_back(std::move(sampler));
	}

	int i = 0;
	for (auto& gltfTexture : model.textures)
	{
		vk::Format format = vk::Format::eR8G8B8A8Srgb;

		//no need to remove i from the set as we check every i only one time
		if (normalTextureIndices.find(i++) != normalTextureIndices.end())
			format = vk::Format::eR8G8B8A8Unorm;

		Texture texture;
		texture.setSampler(m_samplers[gltfTexture.sampler]);
		texture.create("res/models/Sponza/glTF/" + model.images[gltfTexture.source].uri, format);
		m_textures.emplace_back(texture);
	}
}

void Model::loadMaterials(tinygltf::Model& model)
{
	if (m_textures.empty())
		return;

	for (auto& gltfMaterial : model.materials)
	{
		size_t lastTextureIndex = m_textures.size() == 0 ? 0 : m_textures.size() - 1;
		auto textureInfo = gltfMaterial.pbrMetallicRoughness;

		int albedoTexIndex			  = (textureInfo.baseColorTexture.index == -1 ? lastTextureIndex : textureInfo.baseColorTexture.index);
		int metallicRoughnessTexIndex = (textureInfo.metallicRoughnessTexture.index == -1 ? lastTextureIndex : textureInfo.metallicRoughnessTexture.index);
		int normalTexIndex			  = (gltfMaterial.normalTexture.index == -1 ? lastTextureIndex : gltfMaterial.normalTexture.index);

		Material material;
		material.albedo = &m_textures[albedoTexIndex];
		material.normal = &m_textures[normalTexIndex];
		material.metallicRoughness = &m_textures[metallicRoughnessTexIndex];
		material.emissive = nullptr;
		material.occlusion = nullptr;
		m_materials.emplace_back(material);
	}

	Log::info("mat size: {}", m_materials.size());
}

void Model::loadNode(tinygltf::Model& model, tinygltf::Node& node)
{
	if (node.translation.size() == 3)
	{
		glm::vec3 translation = glm::make_vec3(node.translation.data());
		m_translation = glm::translate(glm::mat4(1.0f), translation);
	}

	if (node.rotation.size() == 4)
		m_rotation = glm::mat4(glm::quat(glm::make_vec4(node.rotation.data())));

	if (node.scale.size() == 3)
	{
		glm::vec3 scale = glm::make_vec3(node.scale.data());
		m_scale = glm::scale(glm::mat4(1.0f), scale);
	}

	if (!node.children.empty())
		for (int i = 0; i < node.children.size(); i++)
			loadNode(model, model.nodes[node.children[i]]);

	m_modelMatrix = m_translation * m_rotation * m_scale;

	if (node.mesh < 0)
	{
		Log::error("node has no mesh");
		return;
	}

	const tinygltf::Mesh mesh = model.meshes[node.mesh];
	for (auto& primitive : mesh.primitives)
	{
		if (primitive.indices < 0)
			continue;

		uint32_t firstIndex = static_cast<uint32_t>(m_indexBuffer.size());
		uint32_t vertexStart = static_cast<uint32_t>(m_vertexBuffer.size());
		uint32_t indexCount = 0;

		//vertices
		const float* positionBuffer = nullptr;
		const float* normalsBuffer = nullptr;
		const float* texCoordsBuffer = nullptr;
		const float* tangentsBuffer = nullptr;
		size_t vertexCount = 0;

		if (primitive.attributes.find("POSITION") != primitive.attributes.end())
		{
			const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("POSITION")->second];
			const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
			positionBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
			vertexCount = accessor.count;
		}

		if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
		{
			const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("NORMAL")->second];
			const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
			normalsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
		}

		if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
		{
			const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
			const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
			texCoordsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
		}

		if (primitive.attributes.find("TANGENT") != primitive.attributes.end())
		{
			const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("TANGENT")->second];
			const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
			tangentsBuffer = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
		}

		for (size_t v = 0; v < vertexCount; v++)
		{
			Vertex vertex;
			vertex.position = glm::make_vec3(&positionBuffer[v * 3]);
			vertex.normal = glm::normalize(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f));
			vertex.texCoord = texCoordsBuffer ? glm::make_vec2(&texCoordsBuffer[v * 2]) : glm::vec2(0.0f);
			vertex.tangent = tangentsBuffer ? glm::make_vec4(&tangentsBuffer[v * 4]) : glm::vec4(0.0f);
			m_vertexBuffer.push_back(vertex);
		}

		//indices
		const tinygltf::Accessor& accessor = model.accessors[primitive.indices];
		const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
		const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

		indexCount += static_cast<uint32_t>(accessor.count);

		switch (accessor.componentType)
		{
		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
		{
			const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
			for (size_t index = 0; index < accessor.count; index++)
				m_indexBuffer.push_back(buf[index] + vertexStart);
			break;
		}
		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
		{
			const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
			for (size_t index = 0; index < accessor.count; index++)
				m_indexBuffer.push_back(buf[index] + vertexStart);
			break;
		}
		case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
		{
			const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
			for (size_t index = 0; index < accessor.count; index++)
				m_indexBuffer.push_back(buf[index] + vertexStart);
			break;
		}
		default:
			Log::error("Index component type, {}, not supported!", accessor.componentType);
			return;
		}

		Primitive prim;
		prim.firstIndex = firstIndex;
		prim.indexCount = indexCount;
		prim.materialIndex = primitive.material;
		m_mesh.push_back(prim);
	}
}

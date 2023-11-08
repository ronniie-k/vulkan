#pragma once

#include "Vertex.h"
#include "framework/image/Texture.h"
#include "framework/image/Sampler.h"
#include "framework/Material.h"

#include <string>
#include <tiny_gltf.h>
#include <glm/glm.hpp>

struct Primitive
{
	uint32_t firstIndex;
	uint32_t indexCount;
	uint32_t materialIndex;
};

class Model
{
public:
	Model();

	void destroy();

	void loadFromDisk(const std::string& filename);

	const std::vector<uint32_t>& getIndexData() const { return m_indexBuffer; }
	const std::vector<Vertex>& getVertexData() const { return m_vertexBuffer; }
	const std::vector<Material>& getMaterials() const { return m_materials; }
	const std::vector<Primitive>& getMesh() const { return m_mesh; }
	const glm::mat4 getScale() const { return m_scale; }
	const glm::mat4 getRotation() const { return m_rotation; }
	const glm::mat4 getTranslation() const { return m_translation; }
	const glm::mat4 getModelMatrix() const { return m_modelMatrix; }

private:
	void loadGltfModel(const std::string& filename);
	void loadTextures(tinygltf::Model& model);
	void loadMaterials(tinygltf::Model& model);
	void loadNode(tinygltf::Model& model, tinygltf::Node& node);
private:
	std::vector<std::shared_ptr<Sampler>> m_samplers;
	std::vector<Texture> m_textures;
	std::vector<Material> m_materials;

	std::vector<Primitive> m_mesh;
	std::vector<uint32_t> m_indexBuffer;
	std::vector<Vertex> m_vertexBuffer;


public:
	glm::mat4 m_translation = glm::mat4(1.f);
	glm::mat4 m_rotation = glm::mat4(1.f);
	glm::mat4 m_scale = glm::mat4(1.f);
	glm::mat4 m_modelMatrix = glm::mat4(1.f);
};
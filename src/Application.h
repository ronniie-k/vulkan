#pragma once

#include "framework/Renderer.h"

#include "framework/buffer/VertexBuffer.h"
#include "framework/buffer/IndexBuffer.h"
#include "framework/buffer/UniformBuffer.h"

#include "framework/rendering/DescriptorSet.h"

#include "framework/image/Texture.h"

#include "Vertex.h"
#include "framework/Camera.h"
#include "framework/model/Model.h"

class Application
{
public:
	Application();
	~Application();

	void run();
	void doFrame();
	void updateUniforms();
private:
	void setupDescriptors();
private:
	VertexBuffer<Vertex> m_vertexBuffer;
	IndexBuffer m_indexBuffer;
	std::vector<UniformBuffer> m_uniformBuffers;

	DescriptorSet m_descriptorSet;
	DescriptorSet m_materialDescriptorSet;
	Pipeline m_pipeline;
	RenderPass m_renderPass;
	Camera m_camera;
	Model m_model;
};

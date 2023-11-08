#pragma once

#include "RenderPass.h"
#include "../utils/VulkanStructs.h"

class Pipeline
{
public:
	Pipeline() = default;

	void create(RenderPass& renderPass, const std::string& shaderFileLocation, vk::Extent2D swapChainExtent);
	void destroy();
	
	void setVertexDescriptionInfo(const VertexDescription& vertexDescription) { m_vertexDescription = vertexDescription; }
	void addDescriptorLayout(vk::DescriptorSetLayout layout) { m_descriptors.push_back(layout); }

	vk::PipelineLayout getLayout() { return m_layout; }
	vk::Pipeline handle;
private:
	vk::ShaderModule loadShader(const std::string& filename);
private:
	vk::PipelineLayout m_layout;
	VertexDescription m_vertexDescription;

	std::vector<vk::DescriptorSetLayout> m_descriptors = {};
};
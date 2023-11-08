#pragma once

#include "Buffer.h"

#include "../utils/VulkanStructs.h"
#include "../Renderer.h"

#include <algorithm>

template<typename T>
class VertexBuffer : public Buffer
{
public:
	VertexBuffer() { setUsage(); }
	~VertexBuffer() = default;

	void mapMemory(const std::vector<T>& data)
	{
		void* mappedData;
		vmaMapMemory(Renderer::getAllocator(), m_allocation, &mappedData);
		memcpy(mappedData, data.data(), data.size() * sizeof(data[0]));
		vmaUnmapMemory(Renderer::getAllocator(), m_allocation);
	}

	VertexDescription getVertexDescriptionInfo()
	{
		return { getBindingDescription(), getVertexAttributeDescriptions() };
	}

private:
	vk::VertexInputBindingDescription getBindingDescription()
	{
		vk::VertexInputBindingDescription bindingDescription;
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(T);
		bindingDescription.inputRate = vk::VertexInputRate::eVertex;
		return bindingDescription;
	}
	std::vector<vk::VertexInputAttributeDescription> getVertexAttributeDescriptions()
	{
		std::vector<vk::VertexInputAttributeDescription> attributeDescriptions;
		attributeDescriptions.resize(4);

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[0].offset = offsetof(T, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
		attributeDescriptions[1].offset = offsetof(T, normal);

		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = vk::Format::eR32G32Sfloat;
		attributeDescriptions[2].offset = offsetof(T, texCoord);

		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = vk::Format::eR32G32B32A32Sfloat;
		attributeDescriptions[3].offset = offsetof(T, tangent);

		return attributeDescriptions;
	}
protected:
	void setUsage() { m_usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; }
};
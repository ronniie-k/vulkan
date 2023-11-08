#pragma once

#include "Buffer.h"

#include "../utils/Log.h"
#include "../rendering/Descriptor.h"
#include "../Renderer.h"

class UniformBuffer : public Buffer, public Descriptor
{
public:
	UniformBuffer() { setUsage(); }
	~UniformBuffer() = default;

	template<typename T>
	void mapMemory(const T& data)
	{
		void* mappedData;
		vmaMapMemory(Renderer::getAllocator(), m_allocation, &mappedData);
		memcpy(mappedData, &data, sizeof(T));
		vmaUnmapMemory(Renderer::getAllocator(), m_allocation);
	}

	std::optional<vk::DescriptorBufferInfo> getDescriptorBufferInfo() const
	{
		vk::DescriptorBufferInfo info;
		info.buffer = handle;
		info.offset = 0;
		info.range = m_size;
		return info;
	}

	std::optional<vk::DescriptorImageInfo> getDescriptorImageInfo() const
	{
		return {};
	}

	vk::DescriptorType getDescriptorType() const
	{
		return vk::DescriptorType::eUniformBuffer;
	}

	std::string toString() const
	{
		return "uniform buffer";
	}
protected:
	void setUsage() { m_usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; }
};
#include "Buffer.h"

#include "../utils/Log.h"
#include "../Renderer.h"

void Buffer::create(uint32_t size)
{
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = m_usage;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBuffer vmaHandle = {};
	VkResult success = vmaCreateBuffer(Renderer::getAllocator(), &createInfo, &allocInfo, &vmaHandle, &m_allocation, nullptr);
	if (success != VK_SUCCESS)
	{
		Log::critical("error in Buffer::create(): failed to create buffer");
	}

	handle = vmaHandle;
	m_size = size;
}

void Buffer::destroy()
{
	vmaDestroyBuffer(Renderer::getAllocator(), handle, m_allocation);
}

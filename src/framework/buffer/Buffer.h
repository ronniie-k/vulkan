#pragma once

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

class Buffer
{
public:
	Buffer() = default;
	virtual ~Buffer() = default;

	void create(uint32_t size);
	void destroy();

	uint32_t getSize() { return m_size; }

	vk::Buffer handle;
protected:
	VmaAllocation m_allocation;
	virtual void setUsage() = 0;
	VkBufferUsageFlags m_usage = 0;
	uint32_t m_size = 0;
};
#pragma once

#include "Buffer.h"
#include "../Renderer.h"

class StorageBuffer : public Buffer
{
public:
	StorageBuffer() { setUsage(); }
	~StorageBuffer() = default;

	template<typename T>
	void mapMemory(const T& data)
	{
		void* mappedData;
		vmaMapMemory(Renderer::getAllocator(), m_allocation, &mappedData);
		memcpy(mappedData, &data, sizeof(T));
		vmaUnmapMemory(Renderer::getAllocator(), m_allocation);
	}
protected:
	void setUsage() { m_usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT; }
};
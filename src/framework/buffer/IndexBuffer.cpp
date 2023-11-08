#include "IndexBuffer.h"

#include "../utils/Log.h"
#include "../Renderer.h"

void IndexBuffer::mapMemory(const std::vector<uint32_t>& indices)
{
	m_indexCount = indices.size();
	void* mappedData;
	vmaMapMemory(Renderer::getAllocator(), m_allocation, &mappedData);
	memcpy(mappedData, indices.data(), indices.size() * sizeof(indices[0]));
	vmaUnmapMemory(Renderer::getAllocator(), m_allocation);
}

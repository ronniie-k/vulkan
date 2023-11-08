#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
public:
	IndexBuffer() { setUsage(); }
	~IndexBuffer() = default;

	uint32_t getIndexCount() { return m_indexCount; }
	void mapMemory(const std::vector<uint32_t>& indices);
protected:
	void setUsage() { m_usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; }
	uint32_t m_indexCount = 0;
};
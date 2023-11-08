#pragma once

#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

//only image supported is 2d image
class Image
{
public:
	Image() = default;
	~Image();

	vk::Image getHandle() const { return m_handle; }
	vk::ImageView getView() const { return m_view; }
	vk::ImageLayout getCurrentLayout() const { return m_currentLayout; }

	void setHandle(vk::Image handle) { m_handle = handle; }
	void setView(vk::ImageView view) { m_view = view; }

	void create(uint32_t width, uint32_t height, vk::Format format, vk::Flags<vk::ImageUsageFlagBits> usage);
	void createView(vk::Format format, vk::ImageAspectFlagBits aspectFlags);

	void transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	void destroy();

	uint32_t getWidth() { return m_width; }
	uint32_t getHeight() { return m_height; }
private:
	vk::Device m_device;
	vk::Image m_handle;
	vk::ImageView m_view;
	VmaAllocation m_allocation;
	vk::ImageLayout m_currentLayout = vk::ImageLayout::eUndefined;

	uint32_t m_width;
	uint32_t m_height;
};
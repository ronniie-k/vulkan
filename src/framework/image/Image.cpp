#include "Image.h"

#include "../utils/Log.h"
#include "../utils/Utils.h"
#include "../Renderer.h"

Image::~Image()
{
}

void Image::create(uint32_t width, uint32_t height, vk::Format format, vk::Flags<vk::ImageUsageFlagBits> usage)
{
	VkImageCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.extent = VkExtent3D{ width, height, 1 };
	createInfo.mipLevels = 1;
	createInfo.arrayLayers = 1;
	createInfo.format = static_cast<VkFormat>(format);
	createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.usage = static_cast<VkImageUsageFlags>(usage);
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	//allocate image buffer
	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage vmaHandle = VK_NULL_HANDLE;

	if (vmaCreateImage(Renderer::getAllocator(), &createInfo, &allocInfo, &vmaHandle, &m_allocation, nullptr) != VK_SUCCESS)
		Log::error("error in Image::create(): failed to create image");

	m_handle = vmaHandle;
}

void Image::createView(vk::Format format, vk::ImageAspectFlagBits aspectFlags)
{
	if (!m_handle)
	{
		Log::error("error in Image::createView(): invalid handle");
		return;
	}

	vk::ImageViewCreateInfo createInfo;
	createInfo.image = m_handle;
	createInfo.viewType = vk::ImageViewType::e2D;
	createInfo.format = format;
	createInfo.components.r = vk::ComponentSwizzle::eIdentity;
	createInfo.components.g = vk::ComponentSwizzle::eIdentity;
	createInfo.components.b = vk::ComponentSwizzle::eIdentity;
	createInfo.components.a = vk::ComponentSwizzle::eIdentity;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	m_view = Renderer::getDeviceHandle().createImageView(createInfo);
}

void Image::transitionLayout(vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	auto cmd = Renderer::beginSingleTimeCommand();
	vk::ImageMemoryBarrier barrier;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = m_handle;
	barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal || newLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal) {
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

		if (utils::hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eNone;
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		Log::warn("error in Texture::transitionImageLayout(): unsupported transition");
		return;
	}

	std::vector<vk::ImageMemoryBarrier> barriers = { barrier };
	vk::DependencyFlagBits dependencyFlags = static_cast<vk::DependencyFlagBits>(0);
	cmd.pipelineBarrier(sourceStage, destinationStage, dependencyFlags, {}, {}, barriers);

	Renderer::endSingleTimeCommand(cmd);
	m_currentLayout = newLayout;
}

void Image::destroy()
{
	Renderer::getDeviceHandle().destroyImageView(m_view);

	if (m_allocation != VK_NULL_HANDLE)
		vmaDestroyImage(Renderer::getAllocator(), m_handle, m_allocation);
	else
		Renderer::getDeviceHandle().destroyImage(m_handle);
}

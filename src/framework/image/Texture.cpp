#include "Texture.h"

#include "../utils/Log.h"
#include "../Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

void Texture::create(const std::string& path, vk::Format format)
{
    //load data from disk
    stbi_uc* pixels = stbi_load(path.c_str(), &m_width, &m_height, &m_channels, STBI_rgb_alpha);
    vk::DeviceSize imageSize = m_width * m_height * 4;

    if (!pixels)
        Log::error("error in Texture::create(): failed to load texture file: {}", path);

    VkBuffer stagingBuffer;
    VmaAllocation bufferAllocation;

    //allocate staging buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = imageSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo bufferAllocInfo = {};
    bufferAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

    if (vmaCreateBuffer(Renderer::getAllocator(), &bufferInfo, &bufferAllocInfo, &stagingBuffer, &bufferAllocation, nullptr) != VK_SUCCESS)
        Log::error("error in Texture::create(): failed to create staging buffer");

    //map memory
    void* data;
    vmaMapMemory(Renderer::getAllocator(), bufferAllocation, &data);
    memcpy(data, pixels, imageSize);
    vmaUnmapMemory(Renderer::getAllocator(), bufferAllocation);

    stbi_image_free(pixels);

    //create image
    m_image.create(m_width, m_height, format, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

    m_image.transitionLayout(format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
    copyBufferToImage(stagingBuffer);

    m_image.transitionLayout(format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
    m_image.createView(format, vk::ImageAspectFlagBits::eColor);

    //create sampler
    if (!m_hasSampler)
    {
        m_sampler = std::make_shared<Sampler>();
        m_sampler->create(vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat);
        m_hasSampler = true;
    }

    vmaDestroyBuffer(Renderer::getAllocator(), stagingBuffer, bufferAllocation);
}

void Texture::create(const Image& image)
{
    m_image = image;
    if (!m_hasSampler)
    {
        m_sampler = std::make_shared<Sampler>();
        m_sampler->create(vk::Filter::eNearest, vk::SamplerAddressMode::eRepeat);
        m_hasSampler = true;
    }
}

void Texture::destroy()
{
    m_image.destroy();
}

void Texture::copyBufferToImage(VkBuffer buffer)
{
    vk::BufferImageCopy region;
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = vk::Offset3D{ 0, 0, 0 };
    region.imageExtent = vk::Extent3D{ static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height), 1 };

    std::vector<vk::BufferImageCopy> regions = { region };

    auto cmd = Renderer::beginSingleTimeCommand();
    cmd.copyBufferToImage(buffer, m_image.getHandle(), vk::ImageLayout::eTransferDstOptimal, regions);
    Renderer::endSingleTimeCommand(cmd);
}

std::optional<vk::DescriptorBufferInfo> Texture::getDescriptorBufferInfo() const
{
    return {};
}

std::optional<vk::DescriptorImageInfo> Texture::getDescriptorImageInfo() const
{
    if (!m_image.getView())
    {
        Log::warn("error in Sampler::getDescriptorImageInfo(): sampler has no valid image view");
        return {};
    }
    vk::DescriptorImageInfo info;
    info.imageLayout = m_image.getCurrentLayout();
    info.imageView = m_image.getView();
    info.sampler = m_sampler->handle;
    return info;
}
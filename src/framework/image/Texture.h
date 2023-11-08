#pragma once

#include <vma/vk_mem_alloc.h>
#include <stb_image.h>

#include "Image.h"
#include "Sampler.h"

class Texture : public Descriptor
{
public:
	Texture() = default;
	~Texture() = default;

	void create(const std::string& path, vk::Format format);
	void create(const Image& image);
	void destroy();

	void setSampler(const std::shared_ptr<Sampler>& sampler) { m_sampler = sampler; m_hasSampler = true; }
	Sampler& getSampler() { return *m_sampler; }
	Image& getImage() { return m_image; }


	std::optional<vk::DescriptorBufferInfo> getDescriptorBufferInfo() const;
	std::optional<vk::DescriptorImageInfo> getDescriptorImageInfo() const;
	vk::DescriptorType getDescriptorType() const { return vk::DescriptorType::eCombinedImageSampler; }
private:
	void copyBufferToImage(VkBuffer buffer);
private:
	std::shared_ptr<Sampler> m_sampler;
	Image m_image;

	int m_width;
	int m_height;
	int m_channels;
	bool m_hasSampler = false;
};
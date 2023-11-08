#pragma once

#include <vulkan/vulkan.hpp>
#include <optional>

class Descriptor
{
public:
	Descriptor() = default;
	virtual std::optional<vk::DescriptorBufferInfo> getDescriptorBufferInfo() const = 0;
	virtual std::optional<vk::DescriptorImageInfo> getDescriptorImageInfo() const = 0;
	virtual vk::DescriptorType getDescriptorType() const = 0;
};
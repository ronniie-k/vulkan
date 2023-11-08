#pragma once

#include <vulkan/vulkan.hpp>

class RenderPass
{
public:
	RenderPass() = default;

	void create(vk::Format format);
	void destroy();

	vk::RenderPass handle;
private:
};
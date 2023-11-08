#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/vec2.hpp>

class Framebuffer
{
public:
	void create(vk::RenderPass renderPass, const std::vector<vk::ImageView>& attachments, const glm::vec2& size);
	void destroy();

	vk::Framebuffer handle;
private:
};
#pragma once

#include "../Device.h"

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "../image/Image.h"
#include "../image/Framebuffer.h"

class Swapchain
{
public:
	Swapchain();
	void create();
	void createFramebuffers();
	void recreate();
	void destroy();

	vk::Format getFormat() const { return m_imageFormat; }
	vk::Extent2D getExtent() const { return m_extent; }
	const std::vector<Image>& getImages() { return m_images; }
	const std::vector<Framebuffer>& getFramebuffers() { return m_framebuffers; }

	void setWindow(GLFWwindow* window) { m_window = window; }
	void setRenderPass(vk::RenderPass renderPass) { m_renderPass = renderPass; }

	vk::SwapchainKHR handle;
private:
	vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats);
	vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes);
	vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
private:
	Device* m_device;
	GLFWwindow* m_window;

	vk::RenderPass m_renderPass;
	vk::Format m_imageFormat;
	vk::Extent2D m_extent;
	std::vector<Image> m_images;
	std::vector<Framebuffer> m_framebuffers;
};
#include "Swapchain.h"

#include <limits>

#include "../Renderer.h"

Swapchain::Swapchain()
{
}

void Swapchain::create()
{
	auto device = Renderer::getDevice();
	vk::PhysicalDevice gpu = Renderer::getGpu();
	VkSurfaceKHR surface = Renderer::getSurface();

	std::vector<vk::SurfaceFormatKHR> formats = gpu.getSurfaceFormatsKHR(surface);
	std::vector<vk::PresentModeKHR> presentModes = gpu.getSurfacePresentModesKHR(surface);
	vk::SurfaceCapabilitiesKHR capabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	auto surfaceFormat = chooseSurfaceFormat(formats);
	auto presentMode = choosePresentMode(presentModes);
	m_extent = chooseExtent(capabilities, m_window);

	uint32_t imgCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imgCount > capabilities.maxImageCount)
		imgCount = capabilities.maxImageCount;

	vk::SwapchainCreateInfoKHR createInfo;
	createInfo.surface = surface;
	createInfo.minImageCount = imgCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = m_extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	createInfo.imageSharingMode = vk::SharingMode::eExclusive;

	auto indices = device.findQueueFamilies();
	std::vector<uint32_t> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
		createInfo.queueFamilyIndexCount = queueFamilyIndices.size();
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	handle = device.handle.createSwapchainKHR(createInfo);
	m_imageFormat = surfaceFormat.format;

	auto vulkanImages = device.handle.getSwapchainImagesKHR(handle);
	m_images.resize(vulkanImages.size());

	for (uint32_t i = 0; i < vulkanImages.size(); i++)
	{
		m_images[i].setHandle(vulkanImages[i]);
		m_images[i].createView(m_imageFormat, vk::ImageAspectFlagBits::eColor);
	}
}

void Swapchain::createFramebuffers()
{
	m_framebuffers.resize(m_images.size());

	for (int i = 0; i < m_images.size(); i++)
	{
		std::vector<vk::ImageView> attachments =
		{
			m_images[i].getView(),
			Renderer::getDepthImage().getView(),
		};

		m_framebuffers[i].create(m_renderPass, attachments, { m_extent.width, m_extent.height });
	}
}

void Swapchain::recreate()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(m_window, &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(m_window, &width, &height);
		glfwWaitEvents();
	}

	Renderer::getDeviceHandle().waitIdle();

	destroy();
	create();
	Renderer::createDepthImage();
	createFramebuffers();
}

void Swapchain::destroy()
{
	Renderer::getDepthImage().destroy();
	for (auto framebuffer : m_framebuffers)
		framebuffer.destroy();
	m_framebuffers.clear();

	//only destroy image views as below line destroys images
	for (auto image : m_images)
		Renderer::getDeviceHandle().destroyImageView(image.getView());
	m_images.clear();

	Renderer::getDeviceHandle().destroySwapchainKHR(handle);
	handle = VK_NULL_HANDLE;
}

vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& formats)
{
	for (const auto& format : formats)
	{
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}

	return formats[0];
}

vk::PresentModeKHR Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& presentModes)
{
	for (const auto& presentMode : presentModes)
	{
		if (presentMode == vk::PresentModeKHR::eMailbox)
			return presentMode;
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow* window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		return capabilities.currentExtent;
	else 
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = 
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

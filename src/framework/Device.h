#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <vma/vk_mem_alloc.h>
#include <vector>

#include "utils/VulkanStructs.h"

class Device
{
public:
	Device(vk::Instance& instance, VkSurfaceKHR& surface);
	~Device() = default;

	vk::PhysicalDevice getGpu() { return m_gpu; }
	VkSurfaceKHR getSurface() { return m_surface; }
	VmaAllocator getAllocator() { return m_allocator; }

	void create(const std::vector<const char*> validationLayers);
	void destroy();

	QueueFamilyIndices findQueueFamilies() { return findQueueFamilies(m_gpu); }
	vk::Format findDepthFormat();

	static constexpr uint32_t maxFramesInFlight = 2;
	vk::Device handle;
	vk::Queue m_graphicsQueue;
	vk::Queue m_presentQueue;
private:
	void pickPhysicalDevice();
	void createLogicalDevice(const std::vector<const char*> validationLayers);

	bool isDeviceSuitable(vk::PhysicalDevice device);
	bool checkExtensionSupport(vk::PhysicalDevice device);
	bool querySwapchainSupport(vk::PhysicalDevice device);
	bool anisotropySupported(vk::PhysicalDevice device);

	QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);
	vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features);

	void createAllocator();
private:
	vk::Instance& m_instance;
	VkSurfaceKHR& m_surface;
	vk::PhysicalDevice m_gpu;
	VmaAllocator m_allocator;

	std::vector<const char*> m_extensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
};

//constexpr uint32_t HelloTriangle::maxFramesInFlight = 2;
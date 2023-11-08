#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include "Device.h"
#include "debug/DebugMessenger.h"
#include "swapchain/Swapchain.h"
#include "rendering/RenderPass.h"
#include "rendering/Pipeline.h"
#include "image/Framebuffer.h"
#include "image/Texture.h"

#include "utils/Singleton.h"

class Renderer : public Singleton<Renderer>
{
public:
	~Renderer();

	static vk::CommandBuffer& prepareFrame();
	static void endFrame();

	static void createSwapchain();
	static void createFramebuffers(RenderPass& renderPass);
	static void createDepthImage();

	static vk::CommandBuffer beginSingleTimeCommand();
	static void endSingleTimeCommand(vk::CommandBuffer commandBuffer);
	static vk::RenderPassBeginInfo beginRenderPass(RenderPass& renderPass, vk::Framebuffer framebuffer);

	static Device& getDevice();
	static vk::Device getDeviceHandle();
	static vk::SurfaceKHR getSurface();
	static vk::PhysicalDevice getGpu();
	static VmaAllocator getAllocator();
	static GLFWwindow* getWindow();

	static Image& getDepthImage();
	static Texture& getDepthTexture();
	static vk::Format getSwapchainFormat();
	static vk::Extent2D getSwapchainExtent();
	static const vk::Framebuffer getCurrentFramebuffer();
	static uint32_t getCurrentFrameIndex();
private:
	Renderer();

	vk::CommandBuffer& prepareFrameImpl();
	void endFrameImpl();

	void createSwapchainImpl();
	void createFramebuffersImpl(RenderPass& renderPass);
	void createDepthImageImpl();

	vk::CommandBuffer beginSingleTimeCommandImpl();
	void endSingleTimeCommandImpl(vk::CommandBuffer commandBuffer);
	vk::RenderPassBeginInfo beginRenderPassImpl(RenderPass& renderPass, vk::Framebuffer framebuffer);

	void initVulkan();
	void initGlfw();

	void createInstance();
	void createSurface();
	void createSyncObjects();

	void cleanup();

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
		app->m_resized = true;
	}
private:
	Device m_device;
	GLFWwindow* m_window;

	uint32_t m_width = 800;
	uint32_t m_height = 600;

	vk::Instance m_instance;
	VkSurfaceKHR m_surface;
	DebugMessenger m_debugMessenger;

	Swapchain m_swapchain;
	uint32_t m_fbIndex = 0;
	Texture m_depthTexture;
	Image m_depthImage;

	vk::CommandPool m_commandPool;
	std::vector<vk::CommandBuffer> m_commandBuffers;
	
	std::vector<vk::Semaphore> m_imageAvailableSemaphores;
	std::vector<vk::Semaphore> m_renderFinishedSemaphores;
	std::vector<vk::Fence> m_inFlightFences;
	uint32_t m_currentFrame = 0;
	bool m_resized = false;

	std::array<vk::ClearValue, 2> m_clearColorValues;
#ifdef _DEBUG
	const bool m_enableValidationLayers = true;
	const std::vector<const char*> m_validationLayers =
	{
		"VK_LAYER_KHRONOS_validation",
	};
#else
	const bool m_enableValidationLayers = false;
	const std::vector<const char*> m_validationLayers = {};
#endif
	friend class Singleton<Renderer>;
};
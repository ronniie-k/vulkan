#include "Device.h"

#include <set>

#include "utils/Log.h"

#define VMA_IMPLEMENTATION
#include "vma/vk_mem_alloc.h"

Device::Device(vk::Instance& instance, VkSurfaceKHR& surface)
	:m_instance(instance), m_surface(surface)
{
}

void Device::create(const std::vector<const char*> validationLayers)
{
	pickPhysicalDevice();
	createLogicalDevice(validationLayers);
	createAllocator();
}

void Device::destroy()
{
	vmaDestroyAllocator(m_allocator);
	handle.destroy();
}

vk::Format Device::findDepthFormat()
{
	return findSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
								 vk::ImageTiling::eOptimal,
								 vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Device::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlagBits features)
{
	for (vk::Format format : candidates)
	{
		vk::FormatProperties props = m_gpu.getFormatProperties(format);
		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	Log::critical("failed to find supported format");

	return {};
}

void Device::pickPhysicalDevice()
{
	auto devices = m_instance.enumeratePhysicalDevices();
	if (devices.empty())
	{
		Log::critical("failed to find gpus with vulkan support");
		return;
	}

	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			m_gpu = device;
			break;
		}
	}

	if (!m_gpu)
	{
		Log::critical("failed to find suitable gpu");
		return;
	}
}

void Device::createLogicalDevice(const std::vector<const char*> validationLayers)
{
	QueueFamilyIndices indices = findQueueFamilies(m_gpu);
	std::vector<float> priorities = { 1.f };
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		vk::DeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.setQueuePriorities(priorities);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	vk::PhysicalDeviceFeatures features;
	features.samplerAnisotropy = VK_TRUE;

	vk::DeviceCreateInfo createInfo;
	createInfo.setQueueCreateInfos(queueCreateInfos);
	createInfo.pEnabledFeatures = &features;
	createInfo.setPEnabledLayerNames(validationLayers);
	createInfo.setPEnabledExtensionNames(m_extensions);

	handle = m_gpu.createDevice(createInfo);

	//after logical device is created we can get the vkQueue objects
	m_graphicsQueue = handle.getQueue(indices.graphicsFamily.value(), 0);
	m_presentQueue = handle.getQueue(indices.presentFamily.value(), 0);
}

bool Device::isDeviceSuitable(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices = findQueueFamilies(device);
	return indices.isComplete() 
		&& checkExtensionSupport(device) 
		&& querySwapchainSupport(device)
		&& anisotropySupported(device);
}

bool Device::checkExtensionSupport(vk::PhysicalDevice device)
{
	auto availableExtensions = device.enumerateDeviceExtensionProperties();
	std::set<std::string> requiredExtensions(m_extensions.begin(), m_extensions.end());

	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	return requiredExtensions.empty();
}

bool Device::querySwapchainSupport(vk::PhysicalDevice device)
{
	auto formats = device.getSurfaceFormatsKHR(m_surface);
	auto presentModes = device.getSurfacePresentModesKHR(m_surface);
	return !formats.empty() && !presentModes.empty();
}

bool Device::anisotropySupported(vk::PhysicalDevice device)
{
	return device.getFeatures().samplerAnisotropy;
}

QueueFamilyIndices Device::findQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;
	auto queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
			indices.graphicsFamily = i;

		if (device.getSurfaceSupportKHR(i, m_surface))
			indices.presentFamily = i;


		if (indices.isComplete())
			break;
		i++;
	}

	return indices;
}

void Device::createAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = m_gpu;
	allocatorInfo.device = handle;
	allocatorInfo.instance = m_instance;
	vmaCreateAllocator(&allocatorInfo, &m_allocator);
}

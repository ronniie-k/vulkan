#pragma once

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <vector>

namespace utils
{
	std::vector<const char*> getGlfwExtensions();
	std::vector<vk::ExtensionProperties> getExtensions();
	void printExtensions();
	bool checkValidationLayerSupport(const std::vector<const char*> validationLayers);

	template<typename T>
	uint32_t vectorsizeof(const std::vector<T>& vec) { return sizeof(T) * vec.size(); }

	bool hasStencilComponent(vk::Format format);
}
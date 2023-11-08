#include "Utils.h"

#include"Log.h"

std::vector<const char*> utils::getGlfwExtensions()
{
	uint32_t count = 0;
	const char** extensions = glfwGetRequiredInstanceExtensions(&count);
	return std::vector<const char*>(extensions, extensions + count);
}

std::vector<vk::ExtensionProperties> utils::getExtensions()
{
	return vk::enumerateInstanceExtensionProperties();
}

void utils::printExtensions()
{
	auto extensions = getExtensions();
	Log::info("--available extensions--");
	for (const auto& ext : extensions)
		Log::info(ext.extensionName);
}

bool utils::checkValidationLayerSupport(const std::vector<const char*> validationLayers)
{
	auto availableLayers = vk::enumerateInstanceLayerProperties();
	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProps : availableLayers)
		{
			if (strcmp(layerName, layerProps.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

bool utils::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}
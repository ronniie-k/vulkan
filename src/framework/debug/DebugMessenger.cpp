#include "DebugMessenger.h"

VkResult DebugMessenger::create(vk::Instance instance, const VkAllocationCallbacks* pAllocator)
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    populateCreateInfo(createInfo);

    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) 
        return func(instance, &createInfo, pAllocator, &m_handle);
    else 
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DebugMessenger::destroy(vk::Instance instance, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
        func(instance, m_handle, pAllocator);
}
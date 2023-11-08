#include "Renderer.h"

#include "utils/Utils.h"
#include "utils/Log.h"

Renderer::Renderer()
    :m_device(m_instance, m_surface)
{
    Log::setLogLevel(spdlog::level::trace);
    initGlfw();
    initVulkan();

    
    m_clearColorValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.f, 0.f, 0.f, 0.f});
    m_clearColorValues[1].depthStencil = vk::ClearDepthStencilValue(1.f, 0.f);
}

void Renderer::initVulkan()
{
    createInstance();
    if (m_enableValidationLayers)
    {
        if (m_debugMessenger.create(m_instance, nullptr))
        {
            Log::critical("failed to set up debug messenger");
        }
    }

    createSurface();
    m_device.create(m_validationLayers);
    createSyncObjects();
}

void Renderer::initGlfw()
{
    if (!glfwInit())
        Log::critical("failed to init GLFW");

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(m_width, m_height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Renderer::createInstance()
{
    if (m_enableValidationLayers && !utils::checkValidationLayerSupport(m_validationLayers))
    {
        Log::warn("validation layers requested but not available");
        return;
    }

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = "Hello Triangle";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 3, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    vk::InstanceCreateInfo createInfo{};
    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

    auto glfwExtensions = utils::getGlfwExtensions();
    if (m_enableValidationLayers)
    {
        glfwExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        DebugMessenger::populateCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else
    {
        createInfo.pNext = nullptr;
    }
    createInfo.pApplicationInfo = &appInfo;
    createInfo.setPEnabledExtensionNames(glfwExtensions);
    createInfo.setPEnabledLayerNames(m_validationLayers);

    m_instance = vk::createInstance(createInfo);

    utils::printExtensions();
}

void Renderer::createSurface()
{
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
    {
        Log::critical("failed to crate window surface");
    }
}

void Renderer::createSyncObjects()
{
    QueueFamilyIndices queueFamilyIndices = m_device.findQueueFamilies();
    vk::CommandPoolCreateInfo createInfo;
    createInfo.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    createInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    m_commandPool = m_device.handle.createCommandPool(createInfo);

    //command buffers
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandBufferCount = Device::maxFramesInFlight;

    m_commandBuffers = m_device.handle.allocateCommandBuffers(allocInfo);

    //sync objects
    m_imageAvailableSemaphores.resize(Device::maxFramesInFlight);
    m_renderFinishedSemaphores.resize(Device::maxFramesInFlight);
    m_inFlightFences.resize(Device::maxFramesInFlight);

    for (int i = 0; i < Device::maxFramesInFlight; i++)
    {
        vk::SemaphoreCreateInfo semaphoreInfo;
        vk::FenceCreateInfo fenceInfo;
        fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

        m_imageAvailableSemaphores[i] = m_device.handle.createSemaphore(semaphoreInfo);
        m_renderFinishedSemaphores[i] = m_device.handle.createSemaphore(semaphoreInfo);
        m_inFlightFences[i] = m_device.handle.createFence(fenceInfo);
    }
}

void Renderer::cleanup()
{
    for (int i = 0; i < Device::maxFramesInFlight; i++)
    {
        m_device.handle.destroySemaphore(m_imageAvailableSemaphores[i]);
        m_device.handle.destroySemaphore(m_renderFinishedSemaphores[i]);
        m_device.handle.destroyFence(m_inFlightFences[i]);
    }
    m_swapchain.destroy();

    m_device.handle.destroyCommandPool(m_commandPool);
    m_device.destroy();

    if (m_enableValidationLayers)
        m_debugMessenger.destroy(m_instance, nullptr);

    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    m_instance.destroy();

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

Renderer::~Renderer()
{
    cleanup();
}

vk::CommandBuffer& Renderer::prepareFrameImpl()
{
    m_device.handle.waitForFences(1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    auto result = m_device.handle.acquireNextImageKHR(m_swapchain.handle, UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame]);
    m_fbIndex = result.value;

    if (result.result == vk::Result::eErrorOutOfDateKHR)
        m_swapchain.recreate();
    else if (result.result != vk::Result::eSuccess && result.result != vk::Result::eSuboptimalKHR)
        Log::error("failed to acquire swap chain image");

    m_device.handle.resetFences(1, &m_inFlightFences[m_currentFrame]);
    m_commandBuffers[m_currentFrame].reset();

    //record command buffer
    vk::CommandBufferBeginInfo beginInfo;
    m_commandBuffers[m_currentFrame].begin(beginInfo);

    vk::Viewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapchain.getExtent().width);
    viewport.height = static_cast<float>(m_swapchain.getExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    m_commandBuffers[m_currentFrame].setViewport(0, 1, &viewport);

    vk::Rect2D scissor;
    scissor.offset = vk::Offset2D(0, 0);
    scissor.extent = m_swapchain.getExtent();
    m_commandBuffers[m_currentFrame].setScissor(0, 1, &scissor);

    return m_commandBuffers[m_currentFrame];
}

void Renderer::endFrameImpl()
{
    m_commandBuffers[m_currentFrame].end();

    vk::SubmitInfo submitInfo;
    std::vector<vk::Semaphore> waitSemaphores = { m_imageAvailableSemaphores[m_currentFrame] };
    std::vector<vk::Semaphore> signalSemaphores = { m_renderFinishedSemaphores[m_currentFrame] };
    std::vector<vk::PipelineStageFlags> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
    submitInfo.setWaitSemaphores(waitSemaphores);
    submitInfo.setSignalSemaphores(signalSemaphores);
    submitInfo.setWaitDstStageMask(waitStages);
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

    m_device.m_graphicsQueue.submit(submitInfo, m_inFlightFences[m_currentFrame]);

    std::vector<vk::SwapchainKHR> swapchains = { m_swapchain.handle };
    vk::PresentInfoKHR presentInfo;
    presentInfo.setWaitSemaphores(signalSemaphores);
    presentInfo.setSwapchains(swapchains);
    presentInfo.pImageIndices = &m_fbIndex;

    bool invalidSwapchain = false;
    vk::Result presentResult;
    try
    {
        presentResult = m_device.m_presentQueue.presentKHR(presentInfo);
    }
    catch (vk::OutOfDateKHRError)
    {
        invalidSwapchain = true;
    }
    if (invalidSwapchain || presentResult == vk::Result::eSuboptimalKHR || m_resized)
    {
        m_resized = false;
        m_swapchain.recreate();
    }
    else if (presentResult != vk::Result::eSuccess)
    {
        Log::error("failed to acquire swap chain image");
    }

    m_currentFrame = (m_currentFrame + 1) % Device::maxFramesInFlight;
}

void Renderer::createSwapchainImpl()
{
    m_swapchain.setWindow(m_window);
    m_swapchain.create();
}

void Renderer::createFramebuffersImpl(RenderPass& renderPass)
{
    m_swapchain.setRenderPass(renderPass.handle);
    m_swapchain.createFramebuffers();
}

void Renderer::createDepthImageImpl()
{
    auto extent = m_swapchain.getExtent();
    auto depthFormat = m_device.findDepthFormat();
    m_depthImage.create(extent.width, extent.height, depthFormat, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled);
    m_depthImage.createView(depthFormat, vk::ImageAspectFlagBits::eDepth);
    m_depthImage.transitionLayout(depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);
    m_depthTexture.create(m_depthImage);
}

vk::CommandBuffer Renderer::beginSingleTimeCommandImpl()
{
    vk::CommandBufferAllocateInfo allocInfo;
    allocInfo.level = vk::CommandBufferLevel::ePrimary;
    allocInfo.commandPool = m_commandPool;
    allocInfo.commandBufferCount = 1;

    auto buffer = m_device.handle.allocateCommandBuffers(allocInfo)[0];

    vk::CommandBufferBeginInfo beginInfo;
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    buffer.begin(beginInfo);
    return buffer;
}

void Renderer::endSingleTimeCommandImpl(vk::CommandBuffer commandBuffer)
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    m_device.m_graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
    m_device.m_graphicsQueue.waitIdle();

    m_device.handle.freeCommandBuffers(m_commandPool, 1, &commandBuffer);
}

vk::RenderPassBeginInfo Renderer::beginRenderPassImpl(RenderPass& renderPass, vk::Framebuffer framebuffer)
{
    std::array<vk::ClearValue, 2> clearColorValues;
    clearColorValues[0].color = vk::ClearColorValue(std::array<float, 4>{0.f, 0.f, 0.f, 0.f});
    clearColorValues[1].depthStencil = vk::ClearDepthStencilValue(1.f, 0.f);

    vk::RenderPassBeginInfo renderPassInfo;
    renderPassInfo.renderPass = renderPass.handle;
    renderPassInfo.framebuffer = framebuffer;
    renderPassInfo.renderArea.offset = vk::Offset2D(0, 0);
    renderPassInfo.renderArea.extent = Renderer::getSwapchainExtent();
    renderPassInfo.setClearValues(m_clearColorValues);

    return renderPassInfo;
}

vk::CommandBuffer& Renderer::prepareFrame()
{
    return get().prepareFrameImpl();
}

void Renderer::endFrame()
{
    get().endFrameImpl();
}

vk::CommandBuffer Renderer::beginSingleTimeCommand()
{
    return get().beginSingleTimeCommandImpl();
}

void Renderer::endSingleTimeCommand(vk::CommandBuffer commandBuffer)
{
    get().endSingleTimeCommandImpl(commandBuffer);
}

vk::RenderPassBeginInfo Renderer::beginRenderPass(RenderPass& renderPass, vk::Framebuffer framebuffer)
{
    return get().beginRenderPassImpl(renderPass, framebuffer);
}

vk::SurfaceKHR Renderer::getSurface()
{
    return get().m_device.getSurface();
}

vk::PhysicalDevice Renderer::getGpu()
{
    return get().m_device.getGpu();
}

Device& Renderer::getDevice()
{
    return get().m_device;
}

vk::Device Renderer::getDeviceHandle()
{
    return get().m_device.handle;
}

VmaAllocator Renderer::getAllocator()
{
    return get().m_device.getAllocator();
}

GLFWwindow* Renderer::getWindow()
{
    return get().m_window;
}

Image& Renderer::getDepthImage()
{
    return get().m_depthImage;
}

Texture& Renderer::getDepthTexture()
{
    return get().m_depthTexture;
}

vk::Format Renderer::getSwapchainFormat()
{
    return get().m_swapchain.getFormat();
}

vk::Extent2D Renderer::getSwapchainExtent()
{
    return get().m_swapchain.getExtent();
}

const vk::Framebuffer Renderer::getCurrentFramebuffer()
{
    return get().m_swapchain.getFramebuffers()[get().m_fbIndex].handle;
}

uint32_t Renderer::getCurrentFrameIndex()
{
    return get().m_currentFrame;
}

void Renderer::createSwapchain()
{
    get().createSwapchainImpl();
}

void Renderer::createFramebuffers(RenderPass& renderPass)
{
    get().createFramebuffersImpl(renderPass);
}

void Renderer::createDepthImage()
{
    get().createDepthImageImpl();
}

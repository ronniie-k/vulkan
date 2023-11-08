#include "Framebuffer.h"

#include "../utils/Log.h"
#include "../Renderer.h"

void Framebuffer::create(vk::RenderPass renderPass, const std::vector<vk::ImageView>& attachments, const glm::vec2& size)
{
    vk::FramebufferCreateInfo createInfo;
    createInfo.renderPass = renderPass;
    createInfo.setAttachments(attachments);
    createInfo.width = size.x;
    createInfo.height = size.y;
    createInfo.layers = 1;

    handle = Renderer::getDeviceHandle().createFramebuffer(createInfo);
}

void Framebuffer::destroy()
{
    Renderer::getDeviceHandle().destroyFramebuffer(handle);
}

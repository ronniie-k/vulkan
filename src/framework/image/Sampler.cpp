#include "Sampler.h"

#include "../Renderer.h"

Sampler::~Sampler()
{
    destroy();
}

void Sampler::create(vk::Filter filter, vk::SamplerAddressMode addressMode)
{
    vk::SamplerCreateInfo samplerInfo;
    samplerInfo.magFilter = filter;
    samplerInfo.minFilter = filter;
    samplerInfo.addressModeU = addressMode;
    samplerInfo.addressModeV = addressMode;
    samplerInfo.addressModeW = addressMode;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = Renderer::getGpu().getProperties().limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    handle = Renderer::getDeviceHandle().createSampler(samplerInfo);
}

void Sampler::destroy()
{
    if (handle)
    {
        Renderer::getDeviceHandle().destroySampler(handle);
        handle = VK_NULL_HANDLE;
    }
}

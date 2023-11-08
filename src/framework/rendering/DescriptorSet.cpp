#include "DescriptorSet.h"

#include "../utils/Log.h"
#include "../Renderer.h"

#include <algorithm>

void DescriptorSet::create()
{
    createLayout();
    createDescriptorPool();
    createDescriptorSets();
}

void DescriptorSet::destroy()
{
    Renderer::getDeviceHandle().destroyDescriptorPool(m_pool);
    Renderer::getDeviceHandle().destroyDescriptorSetLayout(m_layout);
}

void DescriptorSet::addBinding(vk::DescriptorType type, vk::Flags<vk::ShaderStageFlagBits> stage, uint32_t binding, uint32_t count)
{
    //add layout binding
    vk::DescriptorSetLayoutBinding layoutBinding;
    layoutBinding.binding = binding;
    layoutBinding.descriptorType = type;
    layoutBinding.descriptorCount = count;
    layoutBinding.stageFlags = stage;

    m_bindings.emplace_back(layoutBinding);
}

void DescriptorSet::addPoolSize(vk::DescriptorType type, uint32_t count)
{
    vk::DescriptorPoolSize poolSize;
    poolSize.type = type;
    poolSize.descriptorCount = count;

    m_sizes.emplace_back(poolSize);
    m_maxSets = glm::max(m_maxSets, count);
}

void DescriptorSet::createDescriptorPool()
{
    vk::DescriptorPoolCreateInfo createInfo;
    createInfo.setPoolSizes(m_sizes);
    createInfo.maxSets = m_maxSets;

    m_pool = Renderer::getDeviceHandle().createDescriptorPool(createInfo);
}

void DescriptorSet::createLayout()
{
    vk::DescriptorSetLayoutCreateInfo createInfo;
    createInfo.setBindings(m_bindings);

    m_layout = Renderer::getDeviceHandle().createDescriptorSetLayout(createInfo);
}

void DescriptorSet::createDescriptorSets()
{
    m_sets.resize(m_maxSets);
    std::vector<vk::DescriptorSetLayout> layouts(m_maxSets, m_layout);

    vk::DescriptorSetAllocateInfo allocInfo;
    allocInfo.descriptorPool = m_pool;
    allocInfo.setSetLayouts(layouts);

    m_sets = Renderer::getDeviceHandle().allocateDescriptorSets(allocInfo);
}
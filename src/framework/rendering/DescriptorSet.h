#pragma once

#include "../buffer/UniformBuffer.h"
#include "ShaderMatrixInfo.h"

#include <string>

class DescriptorSet
{
public:
    DescriptorSet() = default;
	~DescriptorSet() = default;

	void create();
	void destroy();

    void addBinding(vk::DescriptorType type, vk::Flags<vk::ShaderStageFlagBits> stage, uint32_t binding, uint32_t count = 1);
    void addPoolSize(vk::DescriptorType type, uint32_t count);

    template<typename T>
    void writeDescriptor(const T& descriptor, uint32_t binding, int index);
    /// write descriptor to vk::DescriptorSet i

    template<typename T>
    void writeDescriptor(const T& descriptor, uint32_t binding);
    /// write descriptor to all sets

	vk::DescriptorSet& operator[](uint32_t index) { return m_sets[index]; }
	vk::DescriptorSetLayout getLayout() { return m_layout; }
private:
	void createDescriptorPool();
	void createLayout();
	void createDescriptorSets();
private:
	vk::DescriptorSetLayout m_layout;
	vk::DescriptorPool m_pool;
	std::vector<vk::DescriptorSet> m_sets;

	std::vector<vk::DescriptorSetLayoutBinding> m_bindings;
	std::vector<vk::DescriptorPoolSize> m_sizes;
    uint32_t m_maxSets = 0;
};

template<typename T>
void DescriptorSet::writeDescriptor(const T& descriptor, uint32_t binding, int index)
{
    if (m_sets.size() == 0)
    {
        Log::error("error in Descriptor::addDescriptorWrite(): descriptor set has not been created yet");
        return;
    }

    auto bufferInfo = descriptor.getDescriptorBufferInfo();
    auto imgInfo = descriptor.getDescriptorImageInfo();

    vk::WriteDescriptorSet write;
    write.dstSet = m_sets[index];
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = descriptor.getDescriptorType();
    write.descriptorCount = 1;

    if (imgInfo)
        write.pImageInfo = &imgInfo.value();

    if (bufferInfo)
        write.pBufferInfo = &bufferInfo.value();

    Renderer::getDeviceHandle().updateDescriptorSets(write, {});
    return;
}

template<typename T>
void DescriptorSet::writeDescriptor(const T& descriptor, uint32_t binding)
{
    if (m_sets.size() == 0)
    {
        Log::error("error in Descriptor::addDescriptorWrite(): descriptor set has not been created yet");
        return;
    }

    for (int i = 0; i < m_sets.size(); i++)
    {
        auto bufferInfo = descriptor.getDescriptorBufferInfo();
        auto imgInfo = descriptor.getDescriptorImageInfo();

        vk::WriteDescriptorSet write;
        write.dstSet = m_sets[i];
        write.dstBinding = binding;
        write.dstArrayElement = 0;
        write.descriptorType = descriptor.getDescriptorType();
        write.descriptorCount = 1;

        if (imgInfo)
            write.pImageInfo = &imgInfo.value();

        if (bufferInfo)
            write.pBufferInfo = &bufferInfo.value();

        Renderer::getDeviceHandle().updateDescriptorSets(write, {});
    }
}
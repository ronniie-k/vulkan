#include "Application.h"

#include "framework/utils/Utils.h"
#include "ShaderMatrixInfo.h"

#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

Application::Application()
{
    Renderer::createSwapchain();
    Renderer::createDepthImage();
	m_renderPass.create(Renderer::getSwapchainFormat());
    Renderer::createFramebuffers(m_renderPass);

    m_model.loadFromDisk("res/models/Sponza/glTF/Sponza.gltf");

    m_indexBuffer.create(utils::vectorsizeof(m_model.getIndexData()));
    m_indexBuffer.mapMemory(m_model.getIndexData());
    
    m_vertexBuffer.create(utils::vectorsizeof(m_model.getVertexData()));
    m_vertexBuffer.mapMemory(m_model.getVertexData());

    m_uniformBuffers.resize(Device::maxFramesInFlight);
    for (auto& buffer : m_uniformBuffers)
        buffer.create(sizeof(ShaderMatrixInfo));

    setupDescriptors();

    m_pipeline.setVertexDescriptionInfo(m_vertexBuffer.getVertexDescriptionInfo());
    m_pipeline.addDescriptorLayout(m_descriptorSet.getLayout());
    m_pipeline.addDescriptorLayout(m_materialDescriptorSet.getLayout());
	m_pipeline.create(m_renderPass, "res/shaders/", Renderer::getSwapchainExtent());

    m_camera.getPosition() = { -9.f, 1.f, -0.5f };
    m_camera.getRotation() = { 0.f, 0.f };
    m_camera.updateMatrices();
}

Application::~Application()
{
    m_indexBuffer.destroy();
    m_vertexBuffer.destroy();

    for (auto& buffer : m_uniformBuffers)
        buffer.destroy();

    m_descriptorSet.destroy();
    m_materialDescriptorSet.destroy();
    m_pipeline.destroy();
    m_renderPass.destroy();
    m_model.destroy();
}

void Application::run()
{
    while (!glfwWindowShouldClose(Renderer::getWindow()))
    {
        glfwPollEvents();
        m_camera.input(0.16f);
        doFrame();
    }

    Renderer::getDevice().handle.waitIdle();
}

void Application::doFrame()
{
    vk::DeviceSize offsets[1] = { vk::DeviceSize() };
    //Log::info("x {}, y {}, z {}", m_camera.getPosition().x, m_camera.getPosition().y, m_camera.getPosition().z);
    updateUniforms();

    auto commandBuffer = Renderer::prepareFrame();
    auto renderPassInfo = Renderer::beginRenderPass(m_renderPass, Renderer::getCurrentFramebuffer());

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.handle);

    commandBuffer.bindVertexBuffers(0, 1, &m_vertexBuffer.handle, offsets);
    commandBuffer.bindIndexBuffer(m_indexBuffer.handle, 0, vk::IndexType::eUint32);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline.getLayout(), 0, { m_descriptorSet[Renderer::getCurrentFrameIndex()] }, {});

    auto& mesh = m_model.getMesh();

    for (auto& primitive : mesh)
    {
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline.getLayout(),
            1, { m_materialDescriptorSet[primitive.materialIndex] }, {});

        commandBuffer.drawIndexed(primitive.indexCount, 1, primitive.firstIndex, 0, 0);
    }

    commandBuffer.endRenderPass();

    Renderer::endFrame();
}

void Application::updateUniforms()
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    auto extent = Renderer::getSwapchainExtent();
    auto cameraPos = m_camera.getPosition();
    static glm::vec3 lightPos = cameraPos;

    if (m_camera.updateLightPos)
    {
        lightPos = cameraPos;
        m_camera.updateLightPos = false;
    }

    ShaderMatrixInfo info;
    info.model = m_model.getModelMatrix();
    info.view = m_camera.getViewMatrix();
    info.proj = m_camera.getProjMatrix();
    info.normal = glm::mat3(glm::transpose(glm::inverse(m_model.getModelMatrix())));
    info.lightPos = { lightPos.x, lightPos.y, lightPos.z, 0.0f };
    info.cameraPos = { cameraPos, 0.0f };

    m_uniformBuffers[Renderer::getCurrentFrameIndex()].mapMemory<ShaderMatrixInfo>(info);
}

void Application::setupDescriptors()
{
    m_descriptorSet.addBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0);
    m_descriptorSet.addBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);

    m_descriptorSet.addPoolSize(vk::DescriptorType::eUniformBuffer, m_uniformBuffers.size());
    m_descriptorSet.addPoolSize(vk::DescriptorType::eCombinedImageSampler, 1);

    m_descriptorSet.create();

    for (int i = 0; i < m_uniformBuffers.size(); i++)
        m_descriptorSet.writeDescriptor(m_uniformBuffers[i], 0, i);

    //m_descriptorSet.writeDescriptor(Renderer::getDepthTexture(), 1);

    m_materialDescriptorSet.addBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 0);
    m_materialDescriptorSet.addBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1);
    m_materialDescriptorSet.addBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 2);
    m_materialDescriptorSet.addPoolSize(vk::DescriptorType::eCombinedImageSampler, m_model.getMaterials().size());
    m_materialDescriptorSet.addPoolSize(vk::DescriptorType::eCombinedImageSampler, m_model.getMaterials().size());
    m_materialDescriptorSet.addPoolSize(vk::DescriptorType::eCombinedImageSampler, m_model.getMaterials().size());
    m_materialDescriptorSet.create();

    auto& materials = m_model.getMaterials();
    for (int i = 0; i < materials.size(); i++)
    {
        m_materialDescriptorSet.writeDescriptor(*materials[i].albedo, 0, i);
        m_materialDescriptorSet.writeDescriptor(*materials[i].normal, 1, i);
        m_materialDescriptorSet.writeDescriptor(*materials[i].metallicRoughness, 2, i);
    }
}
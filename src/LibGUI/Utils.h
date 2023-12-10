#pragma once

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

namespace utils
{
    namespace su
    {
        glm::mat4x4 createModelViewProjectionClipMatrix(vk::Extent2D const& extent);

        template <typename Func>
        void oneTimeSubmit(vk::Device const& device, vk::CommandPool const& commandPool, vk::Queue const& queue, Func const& func)
        {
            vk::CommandBuffer commandBuffer =
                device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
            commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            func(commandBuffer);
            commandBuffer.end();
            queue.submit(vk::SubmitInfo(0, nullptr, nullptr, 1, &commandBuffer), nullptr);
            queue.waitIdle();
        }

        template <class T>
        void copyToDevice(vk::Device const& device, vk::DeviceMemory const& deviceMemory, T const* pData, size_t count, vk::DeviceSize stride = sizeof(T))
        {
            assert(sizeof(T) <= stride);
            uint8_t* deviceData = static_cast<uint8_t*>(device.mapMemory(deviceMemory, 0, count * stride));
            if (stride == sizeof(T))
            {
                memcpy(deviceData, pData, count * sizeof(T));
            }
            else
            {
                for (size_t i = 0; i < count; i++)
                {
                    memcpy(deviceData, &pData[i], sizeof(T));
                    deviceData += stride;
                }
            }
            device.unmapMemory(deviceMemory);
        }

        template <class T>
        void copyToDevice(vk::Device const& device, vk::DeviceMemory const& deviceMemory, T const& data)
        {
            copyToDevice<T>(device, deviceMemory, &data, 1);
        }

        template <class T>
        VULKAN_HPP_INLINE constexpr const T& clamp(const T& v, const T& lo, const T& hi)
        {
            return v < lo ? lo : hi < v ? hi : v;
        }


        vk::DeviceMemory allocateDeviceMemory( vk::Device const &                         device,
                                                vk::PhysicalDeviceMemoryProperties const & memoryProperties,
                                                vk::MemoryRequirements const &             memoryRequirements,
                                                vk::MemoryPropertyFlags                    memoryPropertyFlags );

        vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes);
        vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);

        void setImageLayout(
            vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);

    }

    std::vector<std::string> getDeviceExtensions();

    struct BufferData
    {
        BufferData(vk::PhysicalDevice const& physicalDevice,
            vk::Device const& device,
            vk::DeviceSize             size,
            vk::BufferUsageFlags       usage,
            vk::MemoryPropertyFlags    propertyFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

        void clear(vk::Device const& device)
        {
            device.destroyBuffer(buffer);  // to prevent some validation layer warning, the Buffer needs to be destroyed before the bound DeviceMemory
            device.freeMemory(deviceMemory);
        }

        template <typename DataType>
        void upload(vk::Device const& device, DataType const& data) const
        {
            assert((m_propertyFlags & vk::MemoryPropertyFlagBits::eHostCoherent) && (m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible));
            assert(sizeof(DataType) <= m_size);

            void* dataPtr = device.mapMemory(deviceMemory, 0, sizeof(DataType));
            memcpy(dataPtr, &data, sizeof(DataType));
            device.unmapMemory(deviceMemory);
        }

        template <typename DataType>
        void upload(vk::Device const& device, std::vector<DataType> const& data, size_t stride = 0) const
        {
            assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eHostVisible);

            size_t elementSize = stride ? stride : sizeof(DataType);
            assert(sizeof(DataType) <= elementSize);

            copyToDevice(device, deviceMemory, data.data(), data.size(), elementSize);
        }

        template <typename DataType>
        void upload(vk::PhysicalDevice const& physicalDevice,
            vk::Device const& device,
            vk::CommandPool const& commandPool,
            vk::Queue                     queue,
            std::vector<DataType> const& data,
            size_t                        stride) const
        {
            assert(m_usage & vk::BufferUsageFlagBits::eTransferDst);
            assert(m_propertyFlags & vk::MemoryPropertyFlagBits::eDeviceLocal);

            size_t elementSize = stride ? stride : sizeof(DataType);
            assert(sizeof(DataType) <= elementSize);

            size_t dataSize = data.size() * elementSize;
            assert(dataSize <= m_size);

            BufferData stagingBuffer(physicalDevice, device, dataSize, vk::BufferUsageFlagBits::eTransferSrc);
            copyToDevice(device, stagingBuffer.deviceMemory, data.data(), data.size(), elementSize);

            su::oneTimeSubmit(device,
                commandPool,
                queue,
                [&](vk::CommandBuffer const& commandBuffer)
                { commandBuffer.copyBuffer(stagingBuffer.buffer, buffer, vk::BufferCopy(0, 0, dataSize)); });

            stagingBuffer.clear(device);
        }

        vk::Buffer       buffer;
        vk::DeviceMemory deviceMemory;
#if !defined( NDEBUG )
    private:
        vk::DeviceSize          m_size;
        vk::BufferUsageFlags    m_usage;
        vk::MemoryPropertyFlags m_propertyFlags;
#endif
    };

    struct ImageData
    {
        ImageData(vk::PhysicalDevice const& physicalDevice,
                vk::Device const& device,
                vk::Format                 format,
                vk::Extent2D const& extent,
                vk::ImageTiling            tiling,
                vk::ImageUsageFlags        usage,
                vk::ImageLayout            initialLayout,
                vk::MemoryPropertyFlags    memoryProperties,
                vk::ImageAspectFlags       aspectMask);

        void clear(vk::Device const& device)
        {
            device.destroyImageView(imageView);
            device.destroyImage(image);  // the Image should to be destroyed before the bound DeviceMemory is freed
            device.freeMemory(deviceMemory);
        }

        vk::Format       format;
        vk::Image        image;
        vk::DeviceMemory deviceMemory;
        vk::ImageView    imageView;
    };

    struct DepthBufferData : public ImageData
    {
        DepthBufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format, vk::Extent2D const& extent);
    };

    struct SwapChainData
    {
        SwapChainData(vk::PhysicalDevice const& physicalDevice = nullptr,
            vk::Device const& device = nullptr,
            vk::SurfaceKHR const& surface = nullptr,
            vk::Extent2D const& extent = vk::Extent2D(0,0),
            vk::ImageUsageFlags        usage = vk::ImageUsageFlags(),
            vk::SwapchainKHR const& oldSwapChain = nullptr,
            uint32_t                   graphicsFamilyIndex = 0,
            uint32_t                   presentFamilyIndex = 0);

        void clear(vk::Device const& device)
        {
            for (auto& imageView : imageViews)
            {
                device.destroyImageView(imageView);
            }
            imageViews.clear();
            images.clear();
            device.destroySwapchainKHR(swapChain);
        }

        vk::Format                 colorFormat;
        vk::SwapchainKHR           swapChain;
        std::vector<vk::Image>     images;
        std::vector<vk::ImageView> imageViews;
    };

    struct TextureData
    {
        TextureData(vk::PhysicalDevice const& physicalDevice,
            vk::Device const& device,
            vk::Extent2D const& extent_ = { 256, 256 },
            vk::ImageUsageFlags        usageFlags = {},
            vk::FormatFeatureFlags     formatFeatureFlags = {},
            bool                       anisotropyEnable = false,
            bool                       forceStaging = false);

        void clear(vk::Device const& device)
        {
            if (stagingBufferData)
            {
                stagingBufferData->clear(device);
            }
            imageData->clear(device);
            device.destroySampler(sampler);
        }

        template <typename ImageGenerator>
        void setImage(vk::Device const& device, vk::CommandBuffer const& commandBuffer, ImageGenerator const& imageGenerator)
        {
            void* data = needsStaging
                ? device.mapMemory(stagingBufferData->deviceMemory, 0, device.getBufferMemoryRequirements(stagingBufferData->buffer).size)
                : device.mapMemory(imageData->deviceMemory, 0, device.getImageMemoryRequirements(imageData->image).size);
            imageGenerator(data, extent);
            device.unmapMemory(needsStaging ? stagingBufferData->deviceMemory : imageData->deviceMemory);

            if (needsStaging)
            {
                // Since we're going to blit to the texture image, set its layout to eTransferDstOptimal
                su::setImageLayout(commandBuffer, imageData->image, imageData->format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
                vk::BufferImageCopy copyRegion(0,
                    extent.width,
                    extent.height,
                    vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                    vk::Offset3D(0, 0, 0),
                    vk::Extent3D(extent, 1));
                commandBuffer.copyBufferToImage(stagingBufferData->buffer, imageData->image, vk::ImageLayout::eTransferDstOptimal, copyRegion);
                // Set the layout for the texture image from eTransferDstOptimal to SHADER_READ_ONLY
                su::setImageLayout(
                    commandBuffer, imageData->image, imageData->format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
            }
            else
            {
                // If we can use the linear tiled image as a texture, just do it
                su::setImageLayout(
                    commandBuffer, imageData->image, imageData->format, vk::ImageLayout::ePreinitialized, vk::ImageLayout::eShaderReadOnlyOptimal);
            }
        }

        vk::Format                  format;
        vk::Extent2D                extent;
        bool                        needsStaging;
        std::unique_ptr<BufferData> stagingBufferData;
        std::unique_ptr<ImageData>  imageData;
        vk::Sampler                 sampler;
    };

    vk::DescriptorPool             createDescriptorPool(vk::Device const& device, std::vector<vk::DescriptorPoolSize> const& poolSizes);
    vk::DescriptorSetLayout        createDescriptorSetLayout(vk::Device const& device,
                                                            std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
                                                            vk::DescriptorSetLayoutCreateFlags                                                  flags = {});

    vk::UniqueDevice                createDeviceUnique(vk::PhysicalDevice const& physicalDevice,
                                                        uint32_t                           queueFamilyIndex,
                                                        std::vector<std::string> const& extensions = {},
                                                        vk::PhysicalDeviceFeatures const* physicalDeviceFeatures = nullptr,
                                                        void const* pNext = nullptr);
    std::vector<vk::Framebuffer>   createFramebuffers(vk::Device const& device,
                                                    vk::RenderPass& renderPass,
                                                    std::vector<vk::ImageView> const& imageViews,
                                                    vk::ImageView const& depthImageView,
                                                    vk::Extent2D const& extent);
    vk::Pipeline                   createGraphicsPipeline(vk::Device const& device,
                                                        vk::PipelineCache const& pipelineCache,
                                                        std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
                                                        std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData,
                                                        uint32_t                                                            vertexStride,
                                                        std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset,
                                                        vk::FrontFace                                                       frontFace,
                                                        bool                                                                depthBuffered,
                                                        vk::PipelineLayout const& pipelineLayout,
                                                        vk::RenderPass const& renderPass);
    vk::RenderPass                 createRenderPass(vk::Device const& device,
                                                    vk::Format           colorFormat,
                                                    vk::Format           depthFormat,
                                                    vk::AttachmentLoadOp loadOp = vk::AttachmentLoadOp::eClear,
                                                    vk::ImageLayout      colorFinalLayout = vk::ImageLayout::ePresentSrcKHR);

	uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties);
    std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface);
    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);


    vk::Format           pickDepthFormat(vk::PhysicalDevice const& physicalDevice);
    vk::PresentModeKHR   pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes);
    vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats);

    void                 updateDescriptorSets(vk::Device const& device,
                                                vk::DescriptorSet const& descriptorSet,
                                                std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData,
                                                utils::TextureData const& textureData,
        uint32_t bindingOffset = 0);
    void                 updateDescriptorSets(vk::Device const& device,
                                                vk::DescriptorSet const& descriptorSet,
                                                std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData,
                                                std::vector<utils::TextureData> const& textureData,
                                                uint32_t bindingOffset = 0);
}

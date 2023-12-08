#pragma once

#include <vulkan/vulkan.hpp>

namespace utils
{
    namespace su
    {
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

    }

    std::vector<std::string> getDeviceExtensions();


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

	uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties);
    std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface);
    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask);



}

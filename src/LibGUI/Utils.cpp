#include "pch.h"
#include "Utils.h"

#include <iostream>
#include <numeric>

namespace utils
{
    template <typename TargetType, typename SourceType>
    VULKAN_HPP_INLINE TargetType checked_cast(SourceType value)
    {
        static_assert(sizeof(TargetType) <= sizeof(SourceType), "No need to cast from smaller to larger type!");
        static_assert(std::numeric_limits<SourceType>::is_integer, "Only integer types supported!");
        static_assert(!std::numeric_limits<SourceType>::is_signed, "Only unsigned types supported!");
        static_assert(std::numeric_limits<TargetType>::is_integer, "Only integer types supported!");
        static_assert(!std::numeric_limits<TargetType>::is_signed, "Only unsigned types supported!");
        assert(value <= std::numeric_limits<TargetType>::max());
        return static_cast<TargetType>(value);
    }

    BufferData::BufferData(vk::PhysicalDevice const& physicalDevice,
                            vk::Device const& device,
                            vk::DeviceSize             size,
                            vk::BufferUsageFlags       usage,
                            vk::MemoryPropertyFlags    propertyFlags)
#if !defined( NDEBUG )
        : m_size(size), m_usage(usage), m_propertyFlags(propertyFlags)
#endif
    {
        buffer = device.createBuffer(vk::BufferCreateInfo(vk::BufferCreateFlags(), size, usage));
        deviceMemory = utils::su::allocateDeviceMemory(device, physicalDevice.getMemoryProperties(), device.getBufferMemoryRequirements(buffer), propertyFlags);
        device.bindBufferMemory(buffer, deviceMemory, 0);
    }

    std::vector<std::string> getDeviceExtensions()
    {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    std::vector<std::string> getInstanceExtensions()
    {
        std::vector<std::string> extensions;
        extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined( VK_USE_PLATFORM_ANDROID_KHR )
        extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_IOS_MVK )
        extensions.push_back(VK_MVK_IOS_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_MACOS_MVK )
        extensions.push_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_MIR_KHR )
        extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_VI_NN )
        extensions.push_back(VK_NN_VI_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_WAYLAND_KHR )
        extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_WIN32_KHR )
        extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XCB_KHR )
        extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XLIB_KHR )
        extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined( VK_USE_PLATFORM_XLIB_XRANDR_EXT )
        extensions.push_back(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME);
#endif
        return extensions;
    }

    ImageData::ImageData(vk::PhysicalDevice const& physicalDevice,
                            vk::Device const& device,
                            vk::Format                 format_,
                            vk::Extent2D const& extent,
                            vk::ImageTiling            tiling,
                            vk::ImageUsageFlags        usage,
                            vk::ImageLayout            initialLayout,
                            vk::MemoryPropertyFlags    memoryProperties,
                            vk::ImageAspectFlags       aspectMask)
        : format(format_)
    {
        vk::ImageCreateInfo imageCreateInfo(vk::ImageCreateFlags(),
            vk::ImageType::e2D,
            format,
            vk::Extent3D(extent, 1),
            1,
            1,
            vk::SampleCountFlagBits::e1,
            tiling,
            usage | vk::ImageUsageFlagBits::eSampled,
            vk::SharingMode::eExclusive,
            {},
            initialLayout);
        image = device.createImage(imageCreateInfo);

        deviceMemory = su::allocateDeviceMemory(device, physicalDevice.getMemoryProperties(), device.getImageMemoryRequirements(image), memoryProperties);

        device.bindImageMemory(image, deviceMemory, 0);

        vk::ImageViewCreateInfo imageViewCreateInfo({}, image, vk::ImageViewType::e2D, format, {}, { aspectMask, 0, 1, 0, 1 });
        imageView = device.createImageView(imageViewCreateInfo);
    }

    DepthBufferData::DepthBufferData(vk::PhysicalDevice const& physicalDevice, vk::Device const& device, vk::Format format, vk::Extent2D const& extent)
                                    : ImageData(physicalDevice,
                                        device,
                                        format,
                                        extent,
                                        vk::ImageTiling::eOptimal,
                                        vk::ImageUsageFlagBits::eDepthStencilAttachment,
                                        vk::ImageLayout::eUndefined,
                                        vk::MemoryPropertyFlagBits::eDeviceLocal,
                                        vk::ImageAspectFlagBits::eDepth)
    {
    }


    SwapChainData::SwapChainData(vk::PhysicalDevice const& physicalDevice,
                                    vk::Device const& device,
                                    vk::SurfaceKHR const& surface,
                                    vk::Extent2D const& extent,
                                    vk::ImageUsageFlags        usage,
                                    vk::SwapchainKHR const& oldSwapChain,
                                    uint32_t                   graphicsQueueFamilyIndex,
                                    uint32_t                   presentQueueFamilyIndex)
    {
        if (surface == nullptr)
            return;

        vk::SurfaceFormatKHR surfaceFormat = su::pickSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(surface));
        colorFormat = surfaceFormat.format;

        vk::SurfaceCapabilitiesKHR surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
        vk::Extent2D               swapchainExtent;
        if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max())
        {
            // If the surface size is undefined, the size is set to the size of the images requested.
            swapchainExtent.width = su::clamp(extent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
            swapchainExtent.height = su::clamp(extent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
        }
        else
        {
            // If the surface size is defined, the swap chain size must match
            swapchainExtent = surfaceCapabilities.currentExtent;
        }
        vk::SurfaceTransformFlagBitsKHR preTransform = (surfaceCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity)
            ? vk::SurfaceTransformFlagBitsKHR::eIdentity
            : surfaceCapabilities.currentTransform;
        vk::CompositeAlphaFlagBitsKHR   compositeAlpha =
            (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePreMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePreMultiplied
            : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::ePostMultiplied) ? vk::CompositeAlphaFlagBitsKHR::ePostMultiplied
            : (surfaceCapabilities.supportedCompositeAlpha & vk::CompositeAlphaFlagBitsKHR::eInherit) ? vk::CompositeAlphaFlagBitsKHR::eInherit
            : vk::CompositeAlphaFlagBitsKHR::eOpaque;
        vk::PresentModeKHR         presentMode = su::pickPresentMode(physicalDevice.getSurfacePresentModesKHR(surface));
        vk::SwapchainCreateInfoKHR swapChainCreateInfo({},
            surface,
            su::clamp(3u, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount),
            colorFormat,
            surfaceFormat.colorSpace,
            swapchainExtent,
            1,
            usage,
            vk::SharingMode::eExclusive,
            {},
            preTransform,
            compositeAlpha,
            presentMode,
            true,
            oldSwapChain);
        if (graphicsQueueFamilyIndex != presentQueueFamilyIndex)
        {
            uint32_t queueFamilyIndices[2] = { graphicsQueueFamilyIndex, presentQueueFamilyIndex };
            // If the graphics and present queues are from different queue families, we either have to explicitly transfer
            // ownership of images between the queues, or we have to create the swapchain with imageSharingMode as
            // vk::SharingMode::eConcurrent
            swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        swapChain = device.createSwapchainKHR(swapChainCreateInfo);

        images = device.getSwapchainImagesKHR(swapChain);

        imageViews.reserve(images.size());
        vk::ImageViewCreateInfo imageViewCreateInfo({}, {}, vk::ImageViewType::e2D, colorFormat, {}, { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 });
        for (auto image : images)
        {
            imageViewCreateInfo.image = image;
            imageViews.push_back(device.createImageView(imageViewCreateInfo));
        }
    }


    TextureData::TextureData(vk::PhysicalDevice const& physicalDevice,
                            vk::Device const& device,
                            vk::Extent2D const& extent_,
                            vk::ImageUsageFlags        usageFlags,
                            vk::FormatFeatureFlags     formatFeatureFlags,
                            bool                       anisotropyEnable,
                            bool                       forceStaging)
                            : format(vk::Format::eR8G8B8A8Unorm), extent(extent_)
    {
        vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);

        formatFeatureFlags |= vk::FormatFeatureFlagBits::eSampledImage;
        needsStaging = forceStaging || ((formatProperties.linearTilingFeatures & formatFeatureFlags) != formatFeatureFlags);
        vk::ImageTiling         imageTiling;
        vk::ImageLayout         initialLayout;
        vk::MemoryPropertyFlags requirements;
        if (needsStaging)
        {
            assert((formatProperties.optimalTilingFeatures & formatFeatureFlags) == formatFeatureFlags);
            stagingBufferData =
                std::unique_ptr<BufferData>(new BufferData(physicalDevice, device, extent.width * extent.height * 4, vk::BufferUsageFlagBits::eTransferSrc));
            imageTiling = vk::ImageTiling::eOptimal;
            usageFlags |= vk::ImageUsageFlagBits::eTransferDst;
            initialLayout = vk::ImageLayout::eUndefined;
        }
        else
        {
            imageTiling = vk::ImageTiling::eLinear;
            initialLayout = vk::ImageLayout::ePreinitialized;
            requirements = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;
        }
        imageData = std::unique_ptr<ImageData>(new ImageData(physicalDevice,
            device,
            format,
            extent,
            imageTiling,
            usageFlags | vk::ImageUsageFlagBits::eSampled,
            initialLayout,
            requirements,
            vk::ImageAspectFlagBits::eColor));

        sampler = device.createSampler(vk::SamplerCreateInfo(vk::SamplerCreateFlags(),
            vk::Filter::eLinear,
            vk::Filter::eLinear,
            vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            vk::SamplerAddressMode::eRepeat,
            0.0f,
            anisotropyEnable,
            16.0f,
            false,
            vk::CompareOp::eNever,
            0.0f,
            0.0f,
            vk::BorderColor::eFloatOpaqueBlack));
    }

    vk::DescriptorPool createDescriptorPool(vk::Device const& device, std::vector<vk::DescriptorPoolSize> const& poolSizes)
    {
        assert(!poolSizes.empty());
        uint32_t maxSets =
            std::accumulate(poolSizes.begin(), poolSizes.end(), 0, [](uint32_t sum, vk::DescriptorPoolSize const& dps) { return sum + dps.descriptorCount; });
        assert(0 < maxSets);

        vk::DescriptorPoolCreateInfo descriptorPoolCreateInfo(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxSets, poolSizes);
        return device.createDescriptorPool(descriptorPoolCreateInfo);
    }

    vk::DescriptorSetLayout createDescriptorSetLayout(vk::Device const& device,
                                                        std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> const& bindingData,
                                                        vk::DescriptorSetLayoutCreateFlags                                                  flags)
    {
        std::vector<vk::DescriptorSetLayoutBinding> bindings(bindingData.size());
        for (size_t i = 0; i < bindingData.size(); i++)
        {
            bindings[i] = vk::DescriptorSetLayoutBinding(
                checked_cast<uint32_t>(i), std::get<0>(bindingData[i]), std::get<1>(bindingData[i]), std::get<2>(bindingData[i]));
        }
        return device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo(flags, bindings));
    }

    vk::UniqueDevice createDeviceUnique(vk::PhysicalDevice         const& physicalDevice,
                                        uint32_t                   queueFamilyIndex,
                                        std::vector<std::string>   const& extensions,
                                        vk::PhysicalDeviceFeatures const* physicalDeviceFeatures,
                                        void                       const* pNext)
    {
        std::vector<char const*> enabledExtensions;
        enabledExtensions.reserve(extensions.size());
        for (auto const& ext : extensions)
        {
            enabledExtensions.push_back(ext.data());
        }

        float                     queuePriority = 0.0f;
        vk::DeviceQueueCreateInfo deviceQueueCreateInfo({}, queueFamilyIndex, 1, &queuePriority);
        vk::DeviceCreateInfo      deviceCreateInfo({}, deviceQueueCreateInfo, {}, enabledExtensions, physicalDeviceFeatures, pNext);

        vk::UniqueDevice device = physicalDevice.createDeviceUnique(deviceCreateInfo);
#if ( VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1 )
        // initialize function pointers for instance
        VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
#endif
        return device;
    }

    std::vector<vk::Framebuffer> createFramebuffers(vk::Device const& device,
                                                    vk::RenderPass& renderPass,
                                                    std::vector<vk::ImageView> const& imageViews,
                                                    vk::ImageView const& depthImageView,
                                                    vk::Extent2D const& extent)
    {
        vk::ImageView attachments[2];
        attachments[1] = depthImageView;

        vk::FramebufferCreateInfo framebufferCreateInfo(
            vk::FramebufferCreateFlags(), renderPass, depthImageView ? 2 : 1, attachments, extent.width, extent.height, 1);
        std::vector<vk::Framebuffer> framebuffers;
        framebuffers.reserve(imageViews.size());
        for (auto const& view : imageViews)
        {
            attachments[0] = view;
            framebuffers.push_back(device.createFramebuffer(framebufferCreateInfo));
        }

        return framebuffers;
    }

    vk::Pipeline createGraphicsPipeline(vk::Device const& device,
                                        vk::PipelineCache const& pipelineCache,
                                        std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& vertexShaderData,
                                        std::pair<vk::ShaderModule, vk::SpecializationInfo const*> const& fragmentShaderData,
                                        uint32_t                                                            vertexStride,
                                        std::vector<std::pair<vk::Format, uint32_t>> const& vertexInputAttributeFormatOffset,
                                        vk::FrontFace                                                       frontFace,
                                        bool                                                                depthBuffered,
                                        vk::PipelineLayout const& pipelineLayout,
                                        vk::RenderPass const& renderPass)
    {
        std::array<vk::PipelineShaderStageCreateInfo, 2> pipelineShaderStageCreateInfos = {
          vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eVertex, vertexShaderData.first, "main", vertexShaderData.second),
          vk::PipelineShaderStageCreateInfo(
            vk::PipelineShaderStageCreateFlags(), vk::ShaderStageFlagBits::eFragment, fragmentShaderData.first, "main", fragmentShaderData.second)
        };

        std::vector<vk::VertexInputAttributeDescription> vertexInputAttributeDescriptions;
        vk::PipelineVertexInputStateCreateInfo           pipelineVertexInputStateCreateInfo;
        vk::VertexInputBindingDescription                vertexInputBindingDescription(0, vertexStride);

        if (0 < vertexStride)
        {
            vertexInputAttributeDescriptions.reserve(vertexInputAttributeFormatOffset.size());
            for (uint32_t i = 0; i < vertexInputAttributeFormatOffset.size(); i++)
            {
                vertexInputAttributeDescriptions.emplace_back(i, 0, vertexInputAttributeFormatOffset[i].first, vertexInputAttributeFormatOffset[i].second);
            }
            pipelineVertexInputStateCreateInfo.setVertexBindingDescriptions(vertexInputBindingDescription);
            pipelineVertexInputStateCreateInfo.setVertexAttributeDescriptions(vertexInputAttributeDescriptions);
        }

        vk::PipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(),
            vk::PrimitiveTopology::eTriangleList);

        vk::PipelineViewportStateCreateInfo pipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(), 1, nullptr, 1, nullptr);

        vk::PipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(),
                                                                                        false,
                                                                                        false,
                                                                                        vk::PolygonMode::eFill,
                                                                                        vk::CullModeFlagBits::eBack,
                                                                                        frontFace,
                                                                                        false,
                                                                                        0.0f,
                                                                                        0.0f,
                                                                                        0.0f,
                                                                                        1.0f);

        vk::PipelineMultisampleStateCreateInfo pipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1);

        vk::StencilOpState                      stencilOpState(vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::StencilOp::eKeep, vk::CompareOp::eAlways);
        vk::PipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo(
            vk::PipelineDepthStencilStateCreateFlags(), depthBuffered, depthBuffered, vk::CompareOp::eLessOrEqual, false, false, stencilOpState, stencilOpState);

        vk::ColorComponentFlags colorComponentFlags(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA);
        vk::PipelineColorBlendAttachmentState pipelineColorBlendAttachmentState(false,
            vk::BlendFactor::eZero,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            vk::BlendFactor::eZero,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            colorComponentFlags);
        vk::PipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo(
            vk::PipelineColorBlendStateCreateFlags(), false, vk::LogicOp::eNoOp, pipelineColorBlendAttachmentState, { { 1.0f, 1.0f, 1.0f, 1.0f } });

        std::array<vk::DynamicState, 2>    dynamicStates = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
        vk::PipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo(vk::PipelineDynamicStateCreateFlags(), dynamicStates);

        vk::GraphicsPipelineCreateInfo graphicsPipelineCreateInfo(vk::PipelineCreateFlags(),
                                                                    pipelineShaderStageCreateInfos,
                                                                    &pipelineVertexInputStateCreateInfo,
                                                                    &pipelineInputAssemblyStateCreateInfo,
                                                                    nullptr,
                                                                    &pipelineViewportStateCreateInfo,
                                                                    &pipelineRasterizationStateCreateInfo,
                                                                    &pipelineMultisampleStateCreateInfo,
                                                                    &pipelineDepthStencilStateCreateInfo,
                                                                    &pipelineColorBlendStateCreateInfo,
                                                                    &pipelineDynamicStateCreateInfo,
                                                                    pipelineLayout,
                                                                    renderPass);

        auto result = device.createGraphicsPipeline(pipelineCache, graphicsPipelineCreateInfo);
        assert(result.result == vk::Result::eSuccess);
        return result.value;
    }

    vk::RenderPass createRenderPass(vk::Device const& device, vk::Format colorFormat, vk::Format depthFormat, vk::AttachmentLoadOp loadOp, vk::ImageLayout colorFinalLayout)
    {
        std::vector<vk::AttachmentDescription> attachmentDescriptions;
        assert(colorFormat != vk::Format::eUndefined);
        attachmentDescriptions.emplace_back(vk::AttachmentDescriptionFlags(),
            colorFormat,
            vk::SampleCountFlagBits::e1,
            loadOp,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            colorFinalLayout);
        if (depthFormat != vk::Format::eUndefined)
        {
            attachmentDescriptions.emplace_back(vk::AttachmentDescriptionFlags(),
                depthFormat,
                vk::SampleCountFlagBits::e1,
                loadOp,
                vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::eDepthStencilAttachmentOptimal);
        }
        vk::AttachmentReference colorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::AttachmentReference depthAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
        vk::SubpassDescription  subpassDescription(vk::SubpassDescriptionFlags(),
            vk::PipelineBindPoint::eGraphics,
            {},
            colorAttachment,
            {},
            (depthFormat != vk::Format::eUndefined) ? &depthAttachment : nullptr);
        return device.createRenderPass(vk::RenderPassCreateInfo(vk::RenderPassCreateFlags(), attachmentDescriptions, subpassDescription));
    }

    uint32_t findGraphicsQueueFamilyIndex(std::vector<vk::QueueFamilyProperties> const& queueFamilyProperties)
    {
        // get the first index into queueFamiliyProperties which supports graphics
        std::vector<vk::QueueFamilyProperties>::const_iterator graphicsQueueFamilyProperty =
            std::find_if(queueFamilyProperties.begin(),
                queueFamilyProperties.end(),
                [](vk::QueueFamilyProperties const& qfp) { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
        assert(graphicsQueueFamilyProperty != queueFamilyProperties.end());
        return static_cast<uint32_t>(std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyProperty));
    }

    std::pair<uint32_t, uint32_t> findGraphicsAndPresentQueueFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR const& surface)
    {
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
        assert(queueFamilyProperties.size() < std::numeric_limits<uint32_t>::max());

        uint32_t graphicsQueueFamilyIndex = findGraphicsQueueFamilyIndex(queueFamilyProperties);
        if (physicalDevice.getSurfaceSupportKHR(graphicsQueueFamilyIndex, surface))
        {
            return std::make_pair(graphicsQueueFamilyIndex,
                graphicsQueueFamilyIndex);  // the first graphicsQueueFamilyIndex does also support presents
        }

        // the graphicsQueueFamilyIndex doesn't support present -> look for an other family index that supports both
        // graphics and present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
            {
                return std::make_pair(static_cast<uint32_t>(i), static_cast<uint32_t>(i));
            }
        }

        // there's nothing like a single family index that supports both graphics and present -> look for an other family
        // index that supports present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++)
        {
            if (physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
            {
                return std::make_pair(graphicsQueueFamilyIndex, static_cast<uint32_t>(i));
            }
        }

        throw std::runtime_error("Could not find queues for both graphics or present -> terminating");
    }

    uint32_t findMemoryType(vk::PhysicalDeviceMemoryProperties const& memoryProperties, uint32_t typeBits, vk::MemoryPropertyFlags requirementsMask)
    {
        uint32_t typeIndex = uint32_t(~0);
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
        {
            if ((typeBits & 1) && ((memoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask))
            {
                typeIndex = i;
                break;
            }
            typeBits >>= 1;
        }
        assert(typeIndex != uint32_t(~0));
        return typeIndex;
    }

    vk::Format pickDepthFormat(vk::PhysicalDevice const& physicalDevice)
    {
        std::vector<vk::Format> candidates = { vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint };
        for (vk::Format format : candidates)
        {
            vk::FormatProperties props = physicalDevice.getFormatProperties(format);

            if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
            {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    vk::PresentModeKHR pickPresentMode(std::vector<vk::PresentModeKHR> const& presentModes)
    {
        vk::PresentModeKHR pickedMode = vk::PresentModeKHR::eFifo;
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == vk::PresentModeKHR::eMailbox)
            {
                pickedMode = presentMode;
                break;
            }

            if (presentMode == vk::PresentModeKHR::eImmediate)
            {
                pickedMode = presentMode;
            }
        }
        return pickedMode;
    }

    vk::SurfaceFormatKHR pickSurfaceFormat(std::vector<vk::SurfaceFormatKHR> const& formats)
    {
        assert(!formats.empty());
        vk::SurfaceFormatKHR pickedFormat = formats[0];
        if (formats.size() == 1)
        {
            if (formats[0].format == vk::Format::eUndefined)
            {
                pickedFormat.format = vk::Format::eB8G8R8A8Unorm;
                pickedFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            }
        }
        else
        {
            // request several formats, the first found will be used
            vk::Format        requestedFormats[] = { vk::Format::eB8G8R8A8Unorm, vk::Format::eR8G8B8A8Unorm, vk::Format::eB8G8R8Unorm, vk::Format::eR8G8B8Unorm };
            vk::ColorSpaceKHR requestedColorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
            for (size_t i = 0; i < sizeof(requestedFormats) / sizeof(requestedFormats[0]); i++)
            {
                vk::Format requestedFormat = requestedFormats[i];
                auto       it = std::find_if(formats.begin(),
                    formats.end(),
                    [requestedFormat, requestedColorSpace](vk::SurfaceFormatKHR const& f)
                    { return (f.format == requestedFormat) && (f.colorSpace == requestedColorSpace); });
                if (it != formats.end())
                {
                    pickedFormat = *it;
                    break;
                }
            }
        }
        assert(pickedFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear);
        return pickedFormat;
    }

    void updateDescriptorSets(vk::Device const& device,
        vk::DescriptorSet const& descriptorSet,
        std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData,
        utils::TextureData const& textureData,
        uint32_t                                                                                                        bindingOffset)
    {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(bufferData.size());

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        writeDescriptorSets.reserve(bufferData.size() + 1);
        uint32_t dstBinding = bindingOffset;
        for (auto const& bd : bufferData)
        {
            bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
            writeDescriptorSets.emplace_back(descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back(), &std::get<3>(bd));
        }

        vk::DescriptorImageInfo imageInfo(textureData.sampler, textureData.imageData->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
        writeDescriptorSets.emplace_back(descriptorSet, dstBinding, 0, vk::DescriptorType::eCombinedImageSampler, imageInfo, nullptr, nullptr);

        device.updateDescriptorSets(writeDescriptorSets, nullptr);
    }

    void updateDescriptorSets(vk::Device const& device,
        vk::DescriptorSet const& descriptorSet,
        std::vector<std::tuple<vk::DescriptorType, vk::Buffer const&, vk::DeviceSize, vk::BufferView const&>> const& bufferData,
        std::vector<utils::TextureData> const& textureData,
        uint32_t                                                                                                        bindingOffset)
    {
        std::vector<vk::DescriptorBufferInfo> bufferInfos;
        bufferInfos.reserve(bufferData.size());

        std::vector<vk::WriteDescriptorSet> writeDescriptorSets;
        writeDescriptorSets.reserve(bufferData.size() + (textureData.empty() ? 0 : 1));
        uint32_t dstBinding = bindingOffset;
        for (auto const& bd : bufferData)
        {
            bufferInfos.emplace_back(std::get<1>(bd), 0, std::get<2>(bd));
            writeDescriptorSets.emplace_back(descriptorSet, dstBinding++, 0, 1, std::get<0>(bd), nullptr, &bufferInfos.back(), &std::get<3>(bd));
        }

        std::vector<vk::DescriptorImageInfo> imageInfos;
        if (!textureData.empty())
        {
            imageInfos.reserve(textureData.size());
            for (auto const& td : textureData)
            {
                imageInfos.emplace_back(td.sampler, td.imageData->imageView, vk::ImageLayout::eShaderReadOnlyOptimal);
            }
            writeDescriptorSets.emplace_back(descriptorSet,
                dstBinding,
                0,
                checked_cast<uint32_t>(imageInfos.size()),
                vk::DescriptorType::eCombinedImageSampler,
                imageInfos.data(),
                nullptr,
                nullptr);
        }

        device.updateDescriptorSets(writeDescriptorSets, nullptr);
    }
}
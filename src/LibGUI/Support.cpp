#include "pch.h"
#include "Utils.h"
#include <glm/gtc/matrix_transform.hpp>


namespace utils
{
	namespace su
	{
        glm::mat4x4 createModelViewProjectionClipMatrix(vk::Extent2D const& extent)
        {
            float fov = glm::radians(45.0f);
            if (extent.width > extent.height)
            {
                fov *= static_cast<float>(extent.height) / static_cast<float>(extent.width);
            }

            glm::mat4x4 model = glm::mat4x4(1.0f);
            glm::mat4x4 view = glm::lookAt(glm::vec3(-5.0f, 3.0f, -10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            glm::mat4x4 projection = glm::perspective(fov, 1.0f, 0.1f, 100.0f);
            // clang-format off
            glm::mat4x4 clip = glm::mat4x4(1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, -1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 0.5f, 0.0f,
                0.0f, 0.0f, 0.5f, 1.0f);  // vulkan clip space has inverted y and half z !
            // clang-format on 
            return clip * projection * view * model;
        }

        vk::DeviceMemory allocateDeviceMemory(vk::Device const& device,
            vk::PhysicalDeviceMemoryProperties const& memoryProperties,
            vk::MemoryRequirements const& memoryRequirements,
            vk::MemoryPropertyFlags                    memoryPropertyFlags)
        {
            uint32_t memoryTypeIndex = findMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, memoryPropertyFlags);

            return device.allocateMemory(vk::MemoryAllocateInfo(memoryRequirements.size, memoryTypeIndex));
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

        void setImageLayout(
            vk::CommandBuffer const& commandBuffer, vk::Image image, vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
        {
            vk::AccessFlags sourceAccessMask;
            switch (oldImageLayout)
            {
            case vk::ImageLayout::eTransferDstOptimal: sourceAccessMask = vk::AccessFlagBits::eTransferWrite; break;
            case vk::ImageLayout::ePreinitialized: sourceAccessMask = vk::AccessFlagBits::eHostWrite; break;
            case vk::ImageLayout::eGeneral:  // sourceAccessMask is empty
            case vk::ImageLayout::eUndefined: break;
            default: assert(false); break;
            }

            vk::PipelineStageFlags sourceStage;
            switch (oldImageLayout)
            {
            case vk::ImageLayout::eGeneral:
            case vk::ImageLayout::ePreinitialized: sourceStage = vk::PipelineStageFlagBits::eHost; break;
            case vk::ImageLayout::eTransferDstOptimal: sourceStage = vk::PipelineStageFlagBits::eTransfer; break;
            case vk::ImageLayout::eUndefined: sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;
            default: assert(false); break;
            }

            vk::AccessFlags destinationAccessMask;
            switch (newImageLayout)
            {
            case vk::ImageLayout::eColorAttachmentOptimal: destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal:
                destinationAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
                break;
            case vk::ImageLayout::eGeneral:  // empty destinationAccessMask
            case vk::ImageLayout::ePresentSrcKHR: break;
            case vk::ImageLayout::eShaderReadOnlyOptimal: destinationAccessMask = vk::AccessFlagBits::eShaderRead; break;
            case vk::ImageLayout::eTransferSrcOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferRead; break;
            case vk::ImageLayout::eTransferDstOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferWrite; break;
            default: assert(false); break;
            }

            vk::PipelineStageFlags destinationStage;
            switch (newImageLayout)
            {
            case vk::ImageLayout::eColorAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput; break;
            case vk::ImageLayout::eDepthStencilAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests; break;
            case vk::ImageLayout::eGeneral: destinationStage = vk::PipelineStageFlagBits::eHost; break;
            case vk::ImageLayout::ePresentSrcKHR: destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe; break;
            case vk::ImageLayout::eShaderReadOnlyOptimal: destinationStage = vk::PipelineStageFlagBits::eFragmentShader; break;
            case vk::ImageLayout::eTransferDstOptimal:
            case vk::ImageLayout::eTransferSrcOptimal: destinationStage = vk::PipelineStageFlagBits::eTransfer; break;
            default: assert(false); break;
            }

            vk::ImageAspectFlags aspectMask;
            if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
            {
                aspectMask = vk::ImageAspectFlagBits::eDepth;
                if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
                {
                    aspectMask |= vk::ImageAspectFlagBits::eStencil;
                }
            }
            else
            {
                aspectMask = vk::ImageAspectFlagBits::eColor;
            }

            vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
            vk::ImageMemoryBarrier    imageMemoryBarrier(sourceAccessMask,
                destinationAccessMask,
                oldImageLayout,
                newImageLayout,
                VK_QUEUE_FAMILY_IGNORED,
                VK_QUEUE_FAMILY_IGNORED,
                image,
                imageSubresourceRange);
            return commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
        }
	}
}
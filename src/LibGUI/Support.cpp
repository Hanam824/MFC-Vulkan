#include "pch.h"
#include "Utils.h"

namespace utils
{
	namespace su
	{
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
	}
}
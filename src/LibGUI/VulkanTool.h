#pragma once

#include <vulkan/vulkan.hpp>


#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		/*assert(res == VK_SUCCESS);*/																		\
	}																									\
}
#define VK_CHECK_RESULT_HPP(f)																			\
{																										\
	vk::Result res = (f);																				\
	if (res != vk::Result::eSuccess)																	\
	{																									\
		std::cout << "Fatal : vk::Result is \"" << tools::errorString(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		/*assert(res == vk::Result::eSuccess);*/															\
	}																									\
}																										\

namespace tools
{
	/** @brief Disable message boxes on fatal errors */
	extern bool errorModeSilent;

	/** @brief Returns an error code as a string */
	std::string errorString(VkResult errorCode);

	/** @brief Returns an error code as a string */
	std::string errorString(vk::Result errorCode);
}
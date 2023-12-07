
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.hpp>

#include "ViewBase.h"


#include <iostream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <optional>
#include <set>


struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

class AFX_EXT_CLASS CVulkanView : public CViewBase
{
	DECLARE_DYNCREATE(CVulkanView)

public:
	CVulkanView();
	virtual ~CVulkanView();

public:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()


    void drawFrame();

protected:
    void createInstance();
    void setupDebugCallback();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapChain();
    void  createImageViews();
    void createRenderPass();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPool();
    void createCommandBuffers();
    void createSyncObjects();

    void cleanupVulkanAPI();


    // support
	bool checkValidationLayerSupport();
    vk::UniqueShaderModule createShaderModule(const std::vector<char>& code);
    vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes);
    vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
    SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device);
    QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device);

    bool isDeviceSuitable(const vk::PhysicalDevice& device);
    bool checkDeviceExtensionSupport(const vk::PhysicalDevice& device);
protected:
    vk::UniqueInstance m_VkInstance;
    VkDebugUtilsMessengerEXT m_VkDebugMessenger;
    vk::SurfaceKHR surface;

    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueDevice device;

    vk::Queue graphicsQueue;
    vk::Queue presentQueue;

    vk::SwapchainKHR swapChain;
    std::vector<vk::Image> swapChainImages;
    vk::Format swapChainImageFormat;
    vk::Extent2D swapChainExtent;
    std::vector<vk::ImageView> swapChainImageViews;
    std::vector<vk::Framebuffer> swapChainFramebuffers;

    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;

    VkCommandPool commandPool;
    std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    size_t currentFrame = 0;

private:
	vk::ApplicationInfo m_VkAppInfo;
};
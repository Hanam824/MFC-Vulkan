#pragma once

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

#include "Utils.h"




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
    virtual void OnInitialUpdate();
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
    HDC GetHDC() { return m_hDC; };
    HGLRC GetHRC() { return m_hRC; };

protected:
    HGLRC	m_hRC;	// RC Handler
    HDC		m_hDC;	// DC Handler

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
    void createImageViews();
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
    vk::SurfaceKHR m_surface;

    vk::PhysicalDevice m_physicalDevice;
    vk::UniqueDevice m_device;

    vk::Queue m_graphicsQueue;
    vk::Queue m_presentQueue;
    std::pair<uint32_t, uint32_t> m_graphicsAndPresentQueueFamilyIndex;

    //vk::SwapchainKHR swapChain;
    //std::vector<vk::Image> swapChainImages;
    //vk::Format swapChainImageFormat;
    //vk::Extent2D swapChainExtent;
    //std::vector<vk::ImageView> swapChainImageViews;
    //std::vector<vk::Framebuffer> swapChainFramebuffers;

    vk::Extent2D m_extent;
    utils::SwapChainData m_swapChainData;
    std::vector<vk::Framebuffer> m_framebuffers;

    vk::RenderPass renderPass;
    vk::PipelineLayout pipelineLayout;
    vk::Pipeline graphicsPipeline;

    VkCommandPool commandPool;
    //std::vector<vk::CommandBuffer, std::allocator<vk::CommandBuffer>> commandBuffers;
    vk::CommandBuffer commandBuffer;

    std::vector<vk::Semaphore> imageAvailableSemaphores;
    std::vector<vk::Semaphore> renderFinishedSemaphores;
    std::vector<vk::Fence> inFlightFences;
    size_t currentFrame = 0;

private:
	vk::ApplicationInfo m_VkAppInfo;
};
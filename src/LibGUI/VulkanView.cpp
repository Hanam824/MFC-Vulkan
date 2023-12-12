#include "pch.h"

#include <filesystem>

#include "VulkanView.h"
//#include "VulkanTool.h"

const int MAX_FRAMES_IN_FLIGHT = 2;

const uint64_t FenceTimeout = 100000000;

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

static std::vector<char> readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

static std::vector<char> readFile(const std::wstring& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}


IMPLEMENT_DYNCREATE(CVulkanView, CViewBase)

CVulkanView::CVulkanView()
{

}

CVulkanView::~CVulkanView()
{
}

BEGIN_MESSAGE_MAP(CVulkanView, CViewBase)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	//ON_WM_QUERYNEWPALETTE()
	//ON_WM_PALETTECHANGED()
	//ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CVulkanView::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();

	// TODO: add draw code here
	drawFrame();
}

void CVulkanView::OnInitialUpdate()
{
	// TODO: Initialize stuff after the document has been created
	// we need actual size to create swapchain
	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createFramebuffers();
	createCommandPool();
	createCommandBuffer();
	createSyncObjects();
}

// CGLView diagnostics

#ifdef _DEBUG
void CVulkanView::AssertValid() const
{
	CViewBase::AssertValid();
}

#ifndef _WIN32_WCE
void CVulkanView::Dump(CDumpContext& dc) const
{
	CViewBase::Dump(dc);
}
#endif
#endif //_DEBUG


int CVulkanView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CViewBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	m_hDC = ::GetDC(m_hWnd);			// Get the Device context

	//initialize vulkan API
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();

	return 0;
}

void CVulkanView::OnDestroy()
{
	CViewBase::OnDestroy();

	cleanupVulkanAPI();

	::ReleaseDC(m_hWnd, m_hDC);
}

void CVulkanView::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    auto requiredExtension = getRequiredExtensions();
    auto extensions = gatherExtensions(requiredExtension);

    auto layer = gatherLayers(std::vector<std::string>());

    m_VkAppInfo = vk::ApplicationInfo()
        .setPApplicationName("Hello Triangle")
        .setApiVersion(VK_MAKE_VERSION(1, 0, 0))
        .setPEngineName("No Engine")
        .setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
        .setApiVersion(VK_API_VERSION_1_0);

    auto instanceInfo = vk::InstanceCreateInfo()
        .setFlags(vk::InstanceCreateFlags())
        .setPApplicationInfo(&m_VkAppInfo)
        .setPEnabledExtensionNames(extensions)
        .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
        .setPEnabledLayerNames(validationLayers)
        .setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));

    try
    {
        instance = vk::createInstance(instanceInfo);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create instance!");
    }
}

void CVulkanView::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void CVulkanView::createSurface()
{
    surface = instance.createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR()
        .setHinstance(GetModuleHandleA(0))
        .setHwnd(m_hWnd));

}

void CVulkanView::pickPhysicalDevice()
{
    // enumerate the physicalDevices
    auto physicalDevices = instance.enumeratePhysicalDevices();

    if (physicalDevices.size() == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    for (const auto& device : physicalDevices)
    {
        if (isDeviceSuitable(device))
        {
            physicalDevice = device;
            break;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void CVulkanView::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    float queuePriority = 1.0f;
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.push_back(vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(queueFamily)
            .setQueueCount(1)
            .setQueuePriorities(queuePriority));
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};

    auto deviceEx = getDeviceExtensions();
    auto deviceExtensions = gatherExtensions(deviceEx);

    auto createInfo = vk::DeviceCreateInfo()
        .setPQueueCreateInfos(queueCreateInfos.data())
        .setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
        .setPEnabledFeatures(&deviceFeatures)
        .setPEnabledExtensionNames(deviceExtensions)
        .setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()));

    if (enableValidationLayers) 
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else 
    {
        createInfo.enabledLayerCount = 0;
    }

    try
    {
        device = physicalDevice.createDevice(createInfo);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void CVulkanView::createSwapChain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    auto createInfo = vk::SwapchainCreateInfoKHR()
        .setSurface(surface)
        .setMinImageCount(imageCount)
        .setImageFormat(surfaceFormat.format)
        .setImageColorSpace(surfaceFormat.colorSpace)
        .setImageExtent(extent)
        .setImageArrayLayers(1)
        .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)

        .setPreTransform(swapChainSupport.capabilities.currentTransform)
        .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
        .setPresentMode(presentMode)
        .setClipped(vk::True)

        .setOldSwapchain(VK_NULL_HANDLE);

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    try
    {
        swapChain = device.createSwapchainKHR(createInfo);
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

    swapChainImages = device.getSwapchainImagesKHR(swapChain);
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void CVulkanView::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        try
        {
            swapChainImageViews[i] = device.createImageView(vk::ImageViewCreateInfo()
                .setImage(swapChainImages[i])
                .setViewType(vk::ImageViewType::e2D)
                .setFormat(swapChainImageFormat)
                .setComponents(vk::ComponentMapping())
                .setSubresourceRange(vk::ImageSubresourceRange()
                    .setAspectMask(vk::ImageAspectFlagBits::eColor)
                    .setBaseMipLevel(0)
                    .setLevelCount(1)
                    .setBaseArrayLayer(0)
                    .setLayerCount(1))
            );
        }
        catch (const std::exception&)
        {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void CVulkanView::createRenderPass()
{
    auto colorAttachment = vk::AttachmentDescription()
        .setFormat(swapChainImageFormat)
        .setSamples(vk::SampleCountFlagBits::e1)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
        .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
        .setInitialLayout(vk::ImageLayout::eUndefined)
        .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

    auto colorAttachmentRef = vk::AttachmentReference()
        .setAttachment(0)
        .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

    auto subpass = vk::SubpassDescription()
        .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
        .setColorAttachmentCount(1)
        .setColorAttachments(colorAttachmentRef);

    auto dependency = vk::SubpassDependency()
        .setSrcSubpass(VK_SUBPASS_EXTERNAL)
        .setDstSubpass(0)
        .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setSrcAccessMask(vk::AccessFlagBits::eNoneKHR)
        .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
        .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

    try
    {
        renderPass = device.createRenderPass(vk::RenderPassCreateInfo()
            .setAttachmentCount(1)
            .setAttachments(colorAttachment)
            .setSubpassCount(1)
            .setSubpasses(subpass)
            .setDependencyCount(1)
            .setDependencies(dependency));
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}

void CVulkanView::createGraphicsPipeline()
{
    std::filesystem::path currentDirectory = std::filesystem::current_path();

    auto vertShaderCode = readFile(currentDirectory.wstring() + _T("/shaders/vert.spv"));
    auto fragShaderCode = readFile(currentDirectory.wstring() + _T("/shaders/frag.spv"));

    auto vertShaderModule = createShaderModule(vertShaderCode);
    auto fragShaderModule = createShaderModule(fragShaderCode);

    auto vertShaderStageInfo = vk::PipelineShaderStageCreateInfo()
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setModule(vertShaderModule)
        .setPName("main");
    auto fragShaderStageInfo = vk::PipelineShaderStageCreateInfo()
        .setStage(vk::ShaderStageFlagBits::eFragment)
        .setModule(fragShaderModule)
        .setPName("main");

    vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo , fragShaderStageInfo };

    auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount(0)
        .setVertexAttributeDescriptionCount(0);

    auto inputAssembly = vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(vk::False);

    auto viewportState = vk::PipelineViewportStateCreateInfo()
        .setViewportCount(1)
        .setScissorCount(1);

	auto rasterizer = vk::PipelineRasterizationStateCreateInfo()
		.setDepthClampEnable(vk::False)
		.setRasterizerDiscardEnable(vk::False)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.0f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(vk::False);

    auto multisampling = vk::PipelineMultisampleStateCreateInfo()
        .setSampleShadingEnable(vk::False)
        .setRasterizationSamples(vk::SampleCountFlagBits::e1);

    auto colorBlendAttachment = vk::PipelineColorBlendAttachmentState()
        .setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
        .setBlendEnable(vk::False);

    auto colorBlending = vk::PipelineColorBlendStateCreateInfo()
        .setLogicOpEnable(vk::False)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachmentCount(1)
        .setAttachments(colorBlendAttachment)
        .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

    std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

    auto dynamicState = vk::PipelineDynamicStateCreateInfo()
        .setDynamicStateCount(static_cast<uint32_t>(dynamicStates.size()))
        .setDynamicStates(dynamicStates);
    
    try
    {
        pipelineLayout = device.createPipelineLayout(vk::PipelineLayoutCreateInfo()
            .setSetLayoutCount(0)
            .setPushConstantRangeCount(0)
        );
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }

	try
	{
        graphicsPipeline = device.createGraphicsPipeline(VK_NULL_HANDLE, vk::GraphicsPipelineCreateInfo()
            .setStageCount(2)
            .setPStages(shaderStages)
            .setPVertexInputState(&vertexInputInfo)
            .setPInputAssemblyState(&inputAssembly)
            .setPViewportState(&viewportState)
            .setPRasterizationState(&rasterizer)
            .setPMultisampleState(&multisampling)
            .setPColorBlendState(&colorBlending)
            .setPDynamicState(&dynamicState)
            .setLayout(pipelineLayout)
            .setRenderPass(renderPass)
            .setSubpass(0)
            .setBasePipelineHandle(VK_NULL_HANDLE)
            ).value;
	}
	catch (const std::exception&)
	{
        throw std::runtime_error("failed to create graphics pipeline!");
	}

    device.destroyShaderModule(fragShaderModule);
    device.destroyShaderModule(vertShaderModule);
}

void CVulkanView::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++)
    {
        try
        {
            swapChainFramebuffers[i] = device.createFramebuffer(vk::FramebufferCreateInfo()
                .setRenderPass(renderPass)
                .setAttachmentCount(1)
                .setAttachments(swapChainImageViews[i])
                .setWidth(swapChainExtent.width)
                .setHeight(swapChainExtent.height)
                .setLayers(1));
        }
        catch (const std::exception&)
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void CVulkanView::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

    try
    {
        commandPool = device.createCommandPool(vk::CommandPoolCreateInfo()
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
            .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily.value())
        );
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create command pool!");
    }
}

void CVulkanView::createCommandBuffer()
{
    try
    {
        commandBuffer = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo()
            .setCommandPool(commandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary)
            .setCommandBufferCount(1)
        ).front();
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CVulkanView::recordCommandBuffer(vk::CommandBuffer commandBuffer, uint32_t imageIndex)
{
    try
    {
        commandBuffer.begin(vk::CommandBufferBeginInfo());
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

	auto clearColor = vk::ClearValue(vk::ClearColorValue({ 1.0f, 0.8f, 0.4f, 0.0f }));

    auto renderPassInfo = vk::RenderPassBeginInfo()
        .setRenderPass(renderPass)
        .setFramebuffer(swapChainFramebuffers[imageIndex])
        .setRenderArea(vk::Rect2D().setOffset({ 0,0 }).setExtent(swapChainExtent))
        .setClearValueCount(1)
        .setPClearValues(&clearColor);

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    auto viewport = vk::Viewport()
        .setX(0.f)
        .setY(0.f)
        .setWidth(static_cast<float>(swapChainExtent.width))
        .setHeight(static_cast<float>(swapChainExtent.height))
        .setMinDepth(0.0f)
        .setMaxDepth(1.0f);

	commandBuffer.setViewport(0, 1, &viewport);

    auto scissor = vk::Rect2D()
        .setOffset({ 0,0 })
        .setExtent(swapChainExtent);

    commandBuffer.setScissor(0, 1, &scissor);

    commandBuffer.draw(3, 1, 0, 0);

    commandBuffer.endRenderPass();

    try
    {
        commandBuffer.end();
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CVulkanView::createSyncObjects()
{
	try
	{
		imageAvailableSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
		renderFinishedSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
		inFlightFence = device.createFence(vk::FenceCreateInfo().setFlags(vk::FenceCreateFlagBits::eSignaled));
	}
	catch (const std::exception&)
	{
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
}

void CVulkanView::drawFrame()
{
    while (vk::Result::eTimeout == device.waitForFences(inFlightFence, VK_TRUE, FenceTimeout))
        ;

    auto result = device.resetFences(1, &inFlightFence);

    uint32_t imageIndex;
    
    result = device.acquireNextImageKHR(swapChain, FenceTimeout, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

    commandBuffer.reset();

    recordCommandBuffer(commandBuffer, imageIndex);

    vk::Semaphore waitSemaphores[] = { imageAvailableSemaphore };
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput };
    vk::Semaphore signalSemaphores[] = { renderFinishedSemaphore };

    auto submitInfo = vk::SubmitInfo()
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(waitSemaphores)
        .setWaitDstStageMask(waitStages)
        .setCommandBufferCount(1)
        .setPCommandBuffers(&commandBuffer)
        .setSignalSemaphoreCount(1)
        .setPSignalSemaphores(signalSemaphores);

    result = graphicsQueue.submit(1, &submitInfo, inFlightFence);
    if (result != vk::Result::eSuccess)
        throw std::runtime_error("failed to submit draw command buffer!");

    result = presentQueue.presentKHR(vk::PresentInfoKHR()
        .setWaitSemaphoreCount(1)
        .setPWaitSemaphores(signalSemaphores)
        .setSwapchainCount(1)
        .setPSwapchains(&swapChain)
        .setPImageIndices(&imageIndex)
    );
}

void CVulkanView::cleanupVulkanAPI()
{
    device.destroySemaphore(renderFinishedSemaphore);
    device.destroySemaphore(imageAvailableSemaphore);
    device.destroyFence(inFlightFence);

    device.destroyCommandPool(commandPool);

    for (auto framebuffer : swapChainFramebuffers) {
        device.destroyFramebuffer(framebuffer);
    }

    device.destroyPipeline(graphicsPipeline);
    device.destroyPipelineLayout(pipelineLayout);
    device.destroyRenderPass(renderPass);

    for (auto imageView : swapChainImageViews) {
        device.destroyImageView(imageView);
    }

    device.destroySwapchainKHR(swapChain);
    device.destroy();

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }

    instance.destroySurfaceKHR(surface);
    instance.destroy();
}

vk::ShaderModule CVulkanView::createShaderModule(const std::vector<char>& code)
{
    vk::ShaderModule shaderModule{};

    try
    {
        shaderModule = device.createShaderModule(vk::ShaderModuleCreateInfo()
            .setCodeSize(code.size())
            .setPCode(reinterpret_cast<const uint32_t*>(code.data()))
        );
    }
    catch (const std::exception&)
    {
        throw std::runtime_error("failed to create shader module!");
    }


    return shaderModule;
}

vk::SurfaceFormatKHR CVulkanView::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR CVulkanView::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D CVulkanView::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else 
    {
        CRect rectClient;
        GetClientRect(rectClient);

        vk::Extent2D actualExtent(static_cast<uint32_t>(rectClient.Width()), static_cast<uint32_t>(rectClient.Height()));

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

SwapChainSupportDetails CVulkanView::querySwapChainSupport(vk::PhysicalDevice device)
{
    SwapChainSupportDetails details;

    details.capabilities = device.getSurfaceCapabilitiesKHR(surface);

    details.formats = device.getSurfaceFormatsKHR(surface);

    details.presentModes = device.getSurfacePresentModesKHR(surface);

    return details; 
}

bool CVulkanView::isDeviceSuitable(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool CVulkanView::checkDeviceExtensionSupport(vk::PhysicalDevice device)
{ 
    auto deviceExtensions = getDeviceExtensions();
    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    auto availableExtensions = device.enumerateDeviceExtensionProperties();
    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

QueueFamilyIndices CVulkanView::findQueueFamilies(vk::PhysicalDevice device)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = i;
        }

        VkBool32 presentSupport = device.getSurfaceSupportKHR(i, surface);
        if (presentSupport) 
        {
            indices.presentFamily = i;
        }

        if (indices.isComplete())
        {
            break;
        }

        i++;
    }

    return indices;
}

std::vector<std::string> CVulkanView::getDeviceExtensions()
{
    return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
}

std::vector<std::string> CVulkanView::getInstanceExtensions()
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

std::vector<std::string> CVulkanView::getRequiredExtensions()
{
    auto extensions = getInstanceExtensions();

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

std::vector<const char*> CVulkanView::gatherExtensions(const std::vector<std::string>& extensions)
{
    std::vector<const char*> enabledExtensions;
    enabledExtensions.reserve(extensions.size());
    for (auto const& ext : extensions)
    {
        enabledExtensions.push_back(ext.data());
    }

//#ifdef DEBUG
//    if (std::none_of(extensions.begin(), extensions.end(), [](std::string const& extension) { return extension == VK_EXT_DEBUG_UTILS_EXTENSION_NAME; }))
//    {
//        enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//    }
//#endif // DEBUG
    
    return enabledExtensions;
}

std::vector<char const*> CVulkanView::gatherLayers(const std::vector<std::string>& layers)
{
    std::vector<const char*> enabledLayers;
    enabledLayers.reserve(layers.size());
    for (auto const& layer : layers)
    {
        enabledLayers.push_back(layer.data());
    }

#ifdef DEBUG
    // Enable standard validation layer to find as much errors as possible!
    if (std::none_of(layers.begin(), layers.end(), [](std::string const& layer) { return layer == "VK_LAYER_KHRONOS_validation"; }))
    {
        enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
    }
    #endif // DEBUG

    return enabledLayers;
}

bool CVulkanView::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
    {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
        {
			if (strcmp(layerName, layerProperties.layerName) == 0)
            {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
        {
			return false;
		}
	}

	return true;
}

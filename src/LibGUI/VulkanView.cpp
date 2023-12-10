#include "pch.h"

#include <filesystem>

#include "VulkanView.h"
#include "VulkanTool.h"
#include "geometries.hpp"


const int WIDTH = 800;
const int HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> instanceExtensions =
{
	VK_KHR_SURFACE_EXTENSION_NAME,
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	VK_EXT_DEBUG_UTILS_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pCallback) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL fnDebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, pCallbackData->pMessage, -1, wString, 4096);
	OutputDebugString(wString);
	return VK_FALSE;
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
	createCommandBuffers();
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
	setupDebugCallback();
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

void CVulkanView::drawFrame()
{
	//VK_CHECK_RESULT_HPP(m_device->waitForFences(1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max()));
	//VK_CHECK_RESULT_HPP(m_device->resetFences(1, &inFlightFences[currentFrame]));

	//uint32_t imageIndex = m_device->acquireNextImageKHR(m_swapChainData.swapChain, std::numeric_limits<uint64_t>::max(),
	//	imageAvailableSemaphores[currentFrame], nullptr).value;

	//vk::SubmitInfo submitInfo = {};

	//vk::Semaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	//vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	//submitInfo.waitSemaphoreCount = 1;
	//submitInfo.pWaitSemaphores = waitSemaphores;
	//submitInfo.pWaitDstStageMask = waitStages;

	//submitInfo.commandBufferCount = 1;
	//submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	//vk::Semaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	//submitInfo.signalSemaphoreCount = 1;
	//submitInfo.pSignalSemaphores = signalSemaphores;

	//try {
	//	m_graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);
	//}
	//catch (vk::SystemError err) {
	//	throw std::runtime_error("failed to submit draw command buffer!");
	//}

	//vk::PresentInfoKHR presentInfo = {};
	//presentInfo.waitSemaphoreCount = 1;
	//presentInfo.pWaitSemaphores = signalSemaphores;

	//vk::SwapchainKHR swapChains[] = { m_swapChainData.swapChain };
	//presentInfo.swapchainCount = 1;
	//presentInfo.pSwapchains = swapChains;
	//presentInfo.pImageIndices = &imageIndex;
	//presentInfo.pResults = nullptr; // Optional

	//VK_CHECK_RESULT_HPP(m_presentQueue.presentKHR(presentInfo));

	//currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void CVulkanView::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	m_VkAppInfo = vk::ApplicationInfo()
		.setPApplicationName("Hello Triangle")
		.setApiVersion(VK_MAKE_VERSION(1, 0, 0))
		.setPEngineName("No Engine")
		.setEngineVersion(VK_MAKE_VERSION(1, 0, 0))
		.setApiVersion(VK_API_VERSION_1_0);

	auto instanceInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&m_VkAppInfo)
		.setPEnabledExtensionNames(instanceExtensions)
		.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()))
		.setPpEnabledLayerNames(validationLayers.data())
		.setEnabledLayerCount(static_cast<uint32_t>(validationLayers.size()));

	try {
		m_VkInstance = vk::createInstanceUnique(instanceInfo, nullptr);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error("failed to create instance!");
	}
}

void CVulkanView::setupDebugCallback()
{
	if (!enableValidationLayers) return;

	auto createInfo = vk::DebugUtilsMessengerCreateInfoEXT()
		.setFlags(vk::DebugUtilsMessengerCreateFlagsEXT())
		.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
		.setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
		.setPfnUserCallback(fnDebugCallback);

	// NOTE: Vulkan-hpp has methods for this, but they trigger linking errors...
	//m_VkInstance->createDebugUtilsMessengerEXT(createInfo);
	//m_VkInstance->createDebugUtilsMessengerEXTUnique(createInfo);

	// NOTE: reinterpret_cast is also used by vulkan.hpp internally for all these structs
	if (CreateDebugUtilsMessengerEXT(m_VkInstance.get(), reinterpret_cast<const VkDebugUtilsMessengerCreateInfoEXT*>(&createInfo), nullptr, &m_VkDebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug callback!");
	}
}

void CVulkanView::createSurface()
{
	m_surface = m_VkInstance->createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(GetModuleHandleA(0))
		.setHwnd(m_hWnd)
	);
}

void CVulkanView::pickPhysicalDevice()
{
	m_physicalDevice = m_VkInstance->enumeratePhysicalDevices().front();
}

void CVulkanView::createLogicalDevice()
{
	m_graphicsAndPresentQueueFamilyIndex = utils::findGraphicsAndPresentQueueFamilyIndex(m_physicalDevice, m_surface);
	m_device = utils::createDeviceUnique(m_physicalDevice, m_graphicsAndPresentQueueFamilyIndex.first, utils::getDeviceExtensions());

	m_graphicsQueue = m_device->getQueue(m_graphicsAndPresentQueueFamilyIndex.first, 0);
	m_presentQueue = m_device->getQueue(m_graphicsAndPresentQueueFamilyIndex.second, 0);
}

void CVulkanView::createSwapChain()
{
	//SwapChainSupportDetails swapChainSupport = querySwapChainSupport(m_physicalDevice);

	//vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	//vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	//m_extent = chooseSwapExtent(swapChainSupport.capabilities);
	m_extent = vk::Extent2D(500,500);

	//auto graphicsAndPresentQueueFamilyIndex = utils::findGraphicsAndPresentQueueFamilyIndex(m_physicalDevice, m_surface);

	m_swapChainData = utils::SwapChainData(m_physicalDevice,
		m_device.get(),
		m_surface,
		m_extent,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
		{},
		m_graphicsAndPresentQueueFamilyIndex.first,
		m_graphicsAndPresentQueueFamilyIndex.second);

}

void CVulkanView::createImageViews()
{
	/*swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vk::ImageViewCreateInfo createInfo = {};
		createInfo.image = swapChainImages[i];
		createInfo.viewType = vk::ImageViewType::e2D;
		createInfo.format = swapChainImageFormat;
		createInfo.components.r = vk::ComponentSwizzle::eIdentity;
		createInfo.components.g = vk::ComponentSwizzle::eIdentity;
		createInfo.components.b = vk::ComponentSwizzle::eIdentity;
		createInfo.components.a = vk::ComponentSwizzle::eIdentity;
		createInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		try {
			swapChainImageViews[i] = device->createImageView(createInfo);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to create image views!");
		}
	}*/
}

void CVulkanView::createRenderPass()
{
	//vk::AttachmentDescription colorAttachment = {};
	//colorAttachment.format = m_swapChainData.colorFormat;
	//colorAttachment.samples = vk::SampleCountFlagBits::e1;
	//colorAttachment.loadOp = vk::AttachmentLoadOp::eClear;
	//colorAttachment.storeOp = vk::AttachmentStoreOp::eStore;
	//colorAttachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
	//colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
	//colorAttachment.initialLayout = vk::ImageLayout::eUndefined;
	//colorAttachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	//vk::AttachmentReference colorAttachmentRef = {};
	//colorAttachmentRef.attachment = 0;
	//colorAttachmentRef.layout = vk::ImageLayout::eColorAttachmentOptimal;

	//vk::SubpassDescription subpass = {};
	//subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	//subpass.colorAttachmentCount = 1;
	//subpass.pColorAttachments = &colorAttachmentRef;

	//vk::SubpassDependency dependency = {};
	//dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	//dependency.dstSubpass = 0;
	//dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	////dependency.srcAccessMask = 0;
	//dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	//dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

	//vk::RenderPassCreateInfo renderPassInfo = {};
	//renderPassInfo.attachmentCount = 1;
	//renderPassInfo.pAttachments = &colorAttachment;
	//renderPassInfo.subpassCount = 1;
	//renderPassInfo.pSubpasses = &subpass;
	//renderPassInfo.dependencyCount = 1;
	//renderPassInfo.pDependencies = &dependency;

	//try {
	//	renderPass = m_device->createRenderPass(renderPassInfo);
	//}
	//catch (vk::SystemError err) {
	//	throw std::runtime_error("failed to create render pass!");
	//}

	utils::DepthBufferData depthBufferData(m_physicalDevice, m_device.get(), vk::Format::eD16Unorm,m_extent);

	renderPass = utils::createRenderPass(m_device.get(), utils::pickSurfaceFormat(m_physicalDevice.getSurfaceFormatsKHR(m_surface)).format, depthBufferData.format);
}

void CVulkanView::createGraphicsPipeline()
{
	std::filesystem::path currentDirectory = std::filesystem::current_path();

	auto vertShaderCode = readFile(currentDirectory.wstring() + _T("/shaders/vert.spv"));
	auto fragShaderCode = readFile(currentDirectory.wstring() + _T("/shaders/frag.spv"));

	auto vertexShaderModule = createShaderModule(vertShaderCode);
	auto fragmentShaderModule = createShaderModule(fragShaderCode);

	/*vk::PipelineShaderStageCreateInfo shaderStages[] = {
		{
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			*vertShaderModule,
			"main"
		},
		{
			vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			*fragShaderModule,
			"main"
		}
	};*/

	/*vk::PipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	vk::Viewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_extent.width;
	viewport.height = (float)m_extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	vk::Rect2D scissor = {};
	scissor.offset = vk::Offset2D(0, 0);
	scissor.extent = m_extent;

	vk::PipelineViewportStateCreateInfo viewportState = {};
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	vk::PipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = vk::PolygonMode::eFill;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = vk::CullModeFlagBits::eBack;
	rasterizer.frontFace = vk::FrontFace::eClockwise;
	rasterizer.depthBiasEnable = VK_FALSE;

	vk::PipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;

	vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
	colorBlendAttachment.blendEnable = VK_FALSE;

	vk::PipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pushConstantRangeCount = 0;

	try {
		pipelineLayout = m_device->createPipelineLayout(pipelineLayoutInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error("failed to create pipeline layout!");
	}*/

	vk::DescriptorSetLayout descriptorSetLayout =
		utils::createDescriptorSetLayout(m_device.get(), {{vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex}});
	pipelineLayout = m_device->createPipelineLayout(vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(), descriptorSetLayout));

	//vk::GraphicsPipelineCreateInfo pipelineInfo = {};
	//pipelineInfo.stageCount = 2;
	//pipelineInfo.pStages = shaderStages;
	//pipelineInfo.pVertexInputState = &vertexInputInfo;
	//pipelineInfo.pInputAssemblyState = &inputAssembly;
	//pipelineInfo.pViewportState = &viewportState;
	//pipelineInfo.pRasterizationState = &rasterizer;
	//pipelineInfo.pMultisampleState = &multisampling;
	//pipelineInfo.pColorBlendState = &colorBlending;
	//pipelineInfo.layout = pipelineLayout;
	//pipelineInfo.renderPass = renderPass;
	//pipelineInfo.subpass = 0;
	//pipelineInfo.basePipelineHandle = nullptr;

	//try {
	//	graphicsPipeline = m_device->createGraphicsPipeline(nullptr, pipelineInfo).value;
	//}
	//catch (vk::SystemError err) {
	//	throw std::runtime_error("failed to create graphics pipeline!");
	//}

	utils::BufferData vertexBufferData(m_physicalDevice, m_device.get(), sizeof(coloredCubeData), vk::BufferUsageFlagBits::eVertexBuffer);
	utils::su::copyToDevice(m_device.get(), vertexBufferData.deviceMemory, coloredCubeData, sizeof(coloredCubeData) / sizeof(coloredCubeData[0]));

	vk::DescriptorPool            descriptorPool = utils::createDescriptorPool(m_device.get(), {{vk::DescriptorType::eUniformBuffer, 1}});
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(descriptorPool, descriptorSetLayout);
	vk::DescriptorSet             descriptorSet = m_device->allocateDescriptorSets(descriptorSetAllocateInfo).front();

	utils::BufferData uniformBufferData(m_physicalDevice, m_device.get(), sizeof(glm::mat4x4), vk::BufferUsageFlagBits::eUniformBuffer);
	glm::mat4x4        mvpcMatrix = utils::su::createModelViewProjectionClipMatrix(m_extent);
	utils::su::copyToDevice(m_device.get(), uniformBufferData.deviceMemory, mvpcMatrix);

	utils::updateDescriptorSets(m_device.get(), descriptorSet, {{vk::DescriptorType::eUniformBuffer, uniformBufferData.buffer, VK_WHOLE_SIZE, {}}}, {});

	vk::PipelineCache pipelineCache = m_device->createPipelineCache(vk::PipelineCacheCreateInfo());
	graphicsPipeline = utils::createGraphicsPipeline(m_device.get(),
		pipelineCache,
		std::make_pair(vertexShaderModule.get(), nullptr),
		std::make_pair(fragmentShaderModule.get(), nullptr),
		sizeof(coloredCubeData[0]),
		{ { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32B32A32Sfloat, 16 } },
		vk::FrontFace::eClockwise,
		true,
		pipelineLayout,
		renderPass);

}

void CVulkanView::createFramebuffers()
{
	utils::DepthBufferData depthBufferData(m_physicalDevice, m_device.get(), vk::Format::eD16Unorm, m_extent);

	m_framebuffers = utils::createFramebuffers(m_device.get(), renderPass, m_swapChainData.imageViews, depthBufferData.imageView, m_extent);

}

void CVulkanView::createCommandPool()
{
	//QueueFamilyIndices queueFamilyIndices = findQueueFamilies(m_physicalDevice);

	//vk::CommandPoolCreateInfo poolInfo = {};
	//poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	//try {
	//	commandPool = m_device->createCommandPool(poolInfo);
	//}
	//catch (vk::SystemError err) {
	//	throw std::runtime_error("failed to create command pool!");
	//}

	commandPool = m_device->createCommandPool({ {}, m_graphicsAndPresentQueueFamilyIndex.first });
}

void CVulkanView::createCommandBuffers()
{
	/*commandBuffers.resize(m_framebuffers.size());

	vk::CommandBufferAllocateInfo allocInfo = {};
	allocInfo.commandPool = commandPool;
	allocInfo.level = vk::CommandBufferLevel::ePrimary;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	try {
		commandBuffers = m_device->allocateCommandBuffers(allocInfo);
	}
	catch (vk::SystemError err) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		vk::CommandBufferBeginInfo beginInfo = {};
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eSimultaneousUse;

		try {
			commandBuffers[i].begin(beginInfo);
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		vk::RenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = m_framebuffers[i];
		renderPassInfo.renderArea.offset = VkOffset2D(0, 0);
		renderPassInfo.renderArea.extent = m_extent;

		vk::ClearValue clearColor = { std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f } };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		commandBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		commandBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

		commandBuffers[i].draw(3, 1, 0, 0);

		commandBuffers[i].endRenderPass();

		try {
			commandBuffers[i].end();
		}
		catch (vk::SystemError err) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}*/

	commandBuffer = m_device->allocateCommandBuffers(vk::CommandBufferAllocateInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1)).front();
}

void CVulkanView::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	try {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			imageAvailableSemaphores[i] = m_device->createSemaphore({});
			renderFinishedSemaphores[i] = m_device->createSemaphore({});
			inFlightFences[i] = m_device->createFence({ vk::FenceCreateFlagBits::eSignaled });
		}
	}
	catch (vk::SystemError err) {
		throw std::runtime_error("failed to create synchronization objects for a frame!");
	}
}

void CVulkanView::cleanupVulkanAPI()
{
	// NOTE: instance destruction is handled by UniqueInstance, same for device

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		m_device->destroySemaphore(renderFinishedSemaphores[i]);
		m_device->destroySemaphore(imageAvailableSemaphores[i]);
		m_device->destroyFence(inFlightFences[i]);
	}

	m_device->destroyCommandPool(commandPool);

	for (auto framebuffer : m_framebuffers) {
		m_device->destroyFramebuffer(framebuffer);
	}

	m_device->destroyPipeline(graphicsPipeline);
	m_device->destroyPipelineLayout(pipelineLayout);
	m_device->destroyRenderPass(renderPass);

	for (auto imageView : m_swapChainData.imageViews) {
		m_device->destroyImageView(imageView);
	}

	// not using UniqeSwapchain to destroy in correct order - before the surface
	m_device->destroySwapchainKHR(m_swapChainData.swapChain);

	// surface is created by glfw, therefore not using a Unique handle
	m_VkInstance->destroySurfaceKHR(m_surface);

	
	if (enableValidationLayers) {
		//m_VkInstance->destroyDebugUtilsMessengerEXT(m_VkDebugMessenger);
		DestroyDebugUtilsMessengerEXT(m_VkInstance.get(), m_VkDebugMessenger, nullptr);
	}

	// NOTE: instance destruction is handled by UniqueInstance
}

bool CVulkanView::checkValidationLayerSupport()
{
	auto availableLayers = vk::enumerateInstanceLayerProperties();
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
			return false;
	}

	return true;
}

vk::UniqueShaderModule CVulkanView::createShaderModule(const std::vector<char>& code) {
	try {
		return m_device->createShaderModuleUnique({
			vk::ShaderModuleCreateFlags(),
			code.size(),
			reinterpret_cast<const uint32_t*>(code.data())
			});
	}
	catch (vk::SystemError err) {
		throw std::runtime_error("failed to create shader module!");
	}
}

vk::SurfaceFormatKHR CVulkanView::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == vk::Format::eUndefined) {
		return { vk::Format::eB8G8R8A8Unorm, vk::ColorSpaceKHR::eSrgbNonlinear };
	}

	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR CVulkanView::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> availablePresentModes) {
	vk::PresentModeKHR bestMode = vk::PresentModeKHR::eFifo;

	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		}
		else if (availablePresentMode == vk::PresentModeKHR::eImmediate) {
			bestMode = availablePresentMode;
		}
	}

	return bestMode;
}

vk::Extent2D CVulkanView::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		vk::Extent2D actualExtent = { static_cast<uint32_t>(WIDTH), static_cast<uint32_t>(HEIGHT) };

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

SwapChainSupportDetails CVulkanView::querySwapChainSupport(const vk::PhysicalDevice& device) {
	SwapChainSupportDetails details;
	details.capabilities = device.getSurfaceCapabilitiesKHR(m_surface);
	details.formats = device.getSurfaceFormatsKHR(m_surface);
	details.presentModes = device.getSurfacePresentModesKHR(m_surface);

	return details;
}

bool CVulkanView::isDeviceSuitable(const vk::PhysicalDevice& device) {
	QueueFamilyIndices indices = findQueueFamilies(device);

	bool extensionsSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool CVulkanView::checkDeviceExtensionSupport(const vk::PhysicalDevice& device)
{
	auto deviceExtensions = utils::getDeviceExtensions();
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : device.enumerateDeviceExtensionProperties())
	{
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

QueueFamilyIndices CVulkanView::findQueueFamilies(vk::PhysicalDevice device)
{
	QueueFamilyIndices indices;

	auto queueFamilies = device.getQueueFamilyProperties();

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
			indices.graphicsFamily = i;
		}

		if (queueFamily.queueCount > 0 && device.getSurfaceSupportKHR(i, m_surface)) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}

		i++;
	}

	return indices;
}
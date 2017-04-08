#include "HelloTriangleApplication.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <SPIRV\GlslangToSpv.h>

#include "cube_data.h"
/* For this sample, we'll start with GLSL so the shader function is plain */
/* and then use the glslang GLSLtoSPV utility to convert it to SPIR-V for */
/* the driver.  We do this for clarity rather than using pre-compiled     */
/* SPIR-V                                                                 */

static const char *vertShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (std140, binding = 0) uniform bufferVals {\n"
"    mat4 mvp;\n"
"} myBufferVals;\n"
"layout (location = 0) in vec4 pos;\n"
"layout (location = 1) in vec4 inColor;\n"
"layout (location = 0) out vec4 outColor;\n"
"out gl_PerVertex { \n"
"    vec4 gl_Position;\n"
"};\n"
"void main() {\n"
"   outColor = inColor;\n"
"   gl_Position = myBufferVals.mvp * pos;\n"
"}\n";

static const char *fragShaderText =
"#version 400\n"
"#extension GL_ARB_separate_shader_objects : enable\n"
"#extension GL_ARB_shading_language_420pack : enable\n"
"layout (location = 0) in vec4 color;\n"
"layout (location = 0) out vec4 outColor;\n"
"void main() {\n"
"   outColor = color;\n"
"}\n";

HelloTriangleApplication::HelloTriangleApplication() :
	width(1280), height(720)
{
}

void HelloTriangleApplication::run() {
	initVulkan();
	mainLoop();
}

void HelloTriangleApplication::terminate() {
	//destroyInstance();

	// Clean up.
	instance.destroySurfaceKHR(surface);
	SDL_DestroyWindow(window);
	SDL_Quit();
	instance.destroy();
}

void HelloTriangleApplication::initVulkan() {
	addInstanceExtensions();
	createInstance();

	addDeviceExtensions();
	selectPhysicalDevice();
	createSurface();
	
	initSwapchainExtension();
	createLogicalDevice();
		
	createCommandPool();
	createCommandBuffer();
	beginCommandBuffer();
	createDeviceQueue();
	
	createSwapchain();
	createImageviews();

	//createDepthBuffer();	//에러가 나는데 왜 나는지는 잘..
	createUniformBuffer();
	createDescriptorPipelineLayouts(false);
	const bool depthPresent = true;
	createRenderpass(depthPresent);
	createShaders(vertShaderText, fragShaderText);

	createFramebuffers(depthPresent);
	createVertexBuffer(g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
						sizeof(g_vb_solid_face_colors_Data[0]), false);
	createDescriptorPool(false);
	createDescriptorSet(false);
	createPipelineCache();
	createPipeline(depthPresent);

	recordCommands();
}

void HelloTriangleApplication::mainLoop() {
	// Poll for user input.
	bool stillRunning = true;
	while (stillRunning) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

			case SDL_QUIT:
				stillRunning = false;
				break;

			default:
				// Do nothing.
				break;
			}
		}

		SDL_Delay(10);
	}
}

void HelloTriangleApplication::addInstanceExtensions()
{
	// Use validation layers if this is a debug build, and use WSI extensions regardless
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
	instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	instanceExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
	instanceExtensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	instanceExtensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	instanceExtensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

}

void HelloTriangleApplication::createInstance() {
	// vk::ApplicationInfo allows the programmer to specifiy some basic information about the
	// program, which can be useful for layers and tools to provide more debug information.
	appInfo = vk::ApplicationInfo()
		.setPApplicationName("Vulkan C++ Windowed Program Template")
		.setApplicationVersion(1)
		.setPEngineName("LunarG SDK")
		.setEngineVersion(1)
		.setApiVersion(VK_API_VERSION_1_0);

	// vk::InstanceCreateInfo is where the programmer specifies the layers and/or extensions that
	// are needed.
	instInfo = vk::InstanceCreateInfo()
		.setFlags(vk::InstanceCreateFlags())
		.setPApplicationInfo(&appInfo)
		.setEnabledExtensionCount(static_cast<uint32_t>(instanceExtensions.size()))
		.setPpEnabledExtensionNames(instanceExtensions.data())
		.setEnabledLayerCount(static_cast<uint32_t>(layers.size()))
		.setPpEnabledLayerNames(layers.data());

	// Create the Vulkan instance.
	try {
		instance = vk::createInstance(instInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a Vulkan instance: " << e.what() << std::endl;
		assert(0 && "Vulkan runtime error.");
	}
}

void HelloTriangleApplication::addDeviceExtensions() {
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void HelloTriangleApplication::createSurface() {
	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		assert(0 && "SDL error.");
	}
	window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL);
	if (window == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		assert(0 && "SDL error.");
	}
	
	// Create a Vulkan surface for rendering
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(window, &windowInfo)) {
		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
	}

	switch (windowInfo.subsystem) {

#if defined(SDL_VIDEO_DRIVER_ANDROID) && defined(VK_USE_PLATFORM_ANDROID_KHR)
	case SDL_SYSWM_ANDROID: {
		vk::AndroidSurfaceCreateInfoKHR surfaceInfo = vk::AndroidSurfaceCreateInfoKHR()
			.setWindow(windowInfo.info.android.window);
		surface = instance.createAndroidSurfaceKHR(surfaceInfo);
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
	case SDL_SYSWM_MIR: {
		vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
			.setConnection(windowInfo.info.mir.connection)
			.setMirSurface(windowInfo.info.mir.surface);
		surface = instance.createMirSurfaceKHR(surfaceInfo);
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
	case SDL_SYSWM_WAYLAND: {
		vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
			.setDisplay(windowInfo.info.wl.display)
			.setSurface(windowInfo.info.wl.surface);
		surface = instance.createWaylandSurfaceKHR(surfaceInfo);
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
	case SDL_SYSWM_WINDOWS: {
		vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
			.setHinstance(GetModuleHandle(NULL))
			.setHwnd(windowInfo.info.win.window);
		surface = instance.createWin32SurfaceKHR(surfaceInfo);
		break;
	}
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
	case SDL_SYSWM_X11: {
		vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
			.setDpy(windowInfo.info.x11.display)
			.setWindow(windowInfo.info.x11.window);
		surface = instance.createXlibSurfaceKHR(surfaceInfo);
		break;
	}
#endif

	default:
		throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
	}
}

void HelloTriangleApplication::selectPhysicalDevice() {
	// enumerate physical devices (GPUs)
	std::vector<vk::PhysicalDevice> gpuList = instance.enumeratePhysicalDevices();
	
	bool found = false;
	for (const auto& device : gpuList) {
		if (isDeviceSuitable(device)) {
			gpu = device;
			found = true;
			break;
		}
	}

	if (!found) {
		std::cout << "Failed to find a suitable GPU!" << std::endl;
		assert(0 && "Vulkan runtime error.");
	}

	gpuMemoryProps = gpu.getMemoryProperties();
	gpuProps = gpu.getProperties();
}

bool HelloTriangleApplication::isDeviceSuitable(vk::PhysicalDevice device) {
	queueProps = device.getQueueFamilyProperties();
	if (queueProps.size() < 1) return false;

	return true;
	/*
	for (unsigned int i = 0; i < queueProps.size(); i++) {
		if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			graphicsFamilyIndex = i;
			return true;
		}
	}
	*/
}

void HelloTriangleApplication::initSwapchainExtension() {
	// Search for a graphics and a present queue in the array of queue
	// families, try to find one that supports both
	graphicsFamilyIndex = UINT32_MAX;
	presentFamilyIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueProps.size(); i++) {
		if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			if (graphicsFamilyIndex == UINT32_MAX) graphicsFamilyIndex = i;

			// Check whether queue supports presenting:
			bool a = gpu.getSurfaceSupportKHR(i, surface);
			if (gpu.getSurfaceSupportKHR(i, surface) == VK_TRUE) {
				graphicsFamilyIndex = i;
				presentFamilyIndex = i;
				break;
			}
		}
	}

	if (presentFamilyIndex == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present, then
		// find a separate present queue.
		for (size_t i = 0; i < queueProps.size(); i++) {
			if (gpu.getSurfaceSupportKHR(i, surface) == VK_TRUE) {
				presentFamilyIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find queues that support graphics
	// and present
	if (graphicsFamilyIndex == UINT32_MAX || presentFamilyIndex == UINT32_MAX) {
		std::cout << "Could not find a queues for both graphics and present";
		assert(0 && "Vulkan runtime error.");
	}

	// Get the list of VkFormats that are supported:
	// Get available surface format
	std::vector<vk::SurfaceFormatKHR> surfFormats = gpu.getSurfaceFormatsKHR(surface);
	if (surfFormats.size() < 1) {
		std::cout << "Failed to find supporting surface formats" << std::endl;
		assert(0 && "Vulkan runtime error.");
	}
	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format.  Otherwise, at least one
	// supported format will be returned.
	if (surfFormats.size() == 1 && surfFormats[0].format == vk::Format::eUndefined) {
		format = vk::Format::eB8G8R8A8Unorm;
	}
	else {
		format = surfFormats[0].format;
	}
}

void HelloTriangleApplication::createLogicalDevice() {

	float queue_priorities[1] = { 0.0 };
	queueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities)
		.setQueueFamilyIndex(graphicsFamilyIndex);

	deviceInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo)
		.setEnabledExtensionCount(deviceExtensions.size())
		.setPpEnabledExtensionNames(deviceExtensions.size()? deviceExtensions.data() : nullptr)
		.setPEnabledFeatures(nullptr);

	try {
		device = gpu.createDevice(deviceInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a logical device: " << e.what() << std::endl;
		assert(0 && "Vulkan runtime error.");
	}
}

void HelloTriangleApplication::createCommandPool() {
	// Create a command pool to allocate our command buffer from
	cmdPoolInfo = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(graphicsFamilyIndex)
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	cmdPool = device.createCommandPool(cmdPoolInfo);
}

void HelloTriangleApplication::createCommandBuffer() {
	// Create the command buffer from the command pool
	cmdInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(cmdPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	cmd = device.allocateCommandBuffers(cmdInfo);
}

void HelloTriangleApplication::beginCommandBuffer() {
	cmdBufInfo = vk::CommandBufferBeginInfo()
		.setPInheritanceInfo(nullptr);

	// Depends on number of CommandBuffer created
	cmd[0].begin(cmdBufInfo);
}

void HelloTriangleApplication::createDeviceQueue() {
	// create 하는게 아니라 get함. device 만들때 이미 만들어짐
	// Depends on createSwapchainExtension
	graphicsQueue = device.getQueue(graphicsFamilyIndex, 0);

	if (graphicsFamilyIndex == presentFamilyIndex) {
		presentQueue = graphicsQueue;
	}
	else {
		presentQueue = device.getQueue(presentFamilyIndex, 0);
	}
}

void HelloTriangleApplication::createSwapchain() {
	// Get surface capabilities
	surfCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	// Present mode
	std::vector<vk::PresentModeKHR> presentModes = gpu.getSurfacePresentModesKHR(surface);
	if (presentModes.size() < 1) {
		std::cout << "Failed to find supporting present mode" << std::endl;
		assert(0 && "Vulkan runtime error.");
	}
	
	vk::Extent2D swapchainExtent;
	// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = width;
		swapchainExtent.height = height;
		if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
			swapchainExtent.width = surfCapabilities.minImageExtent.width;
		}
		else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
			swapchainExtent.width = surfCapabilities.maxImageExtent.width;
		}

		if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
			swapchainExtent.height = surfCapabilities.minImageExtent.height;
		}
		else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
			swapchainExtent.height = surfCapabilities.maxImageExtent.height;
		}
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCapabilities.currentExtent;
	}

	// The FIFO present mode is guaranteed by the spec to be supported
	// Also note that current Android driver only supports FIFO
	vk::PresentModeKHR swapchainPresentMode = vk::PresentModeKHR::eFifo;

	// Determine the number of VkImage's to use in the swap chain.
	// We need to acquire only 1 presentable image at a time.
	// Asking for minImageCount images ensures that we can acquire
	// 1 presentable image as long as we present it before attempting
	// to acquire another.
	uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;
	
	vk::SurfaceTransformFlagBitsKHR preTransform;
	if (surfCapabilities.supportedTransforms & vk::SurfaceTransformFlagBitsKHR::eIdentity) {
		preTransform = vk::SurfaceTransformFlagBitsKHR::eIdentity;
	}
	else {
		preTransform = surfCapabilities.currentTransform;
	}

	// Find a supported composite alpha mode - one of these is guaranteed to be set
	vk::CompositeAlphaFlagBitsKHR compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	vk::CompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::CompositeAlphaFlagBitsKHR::ePreMultiplied,
		vk::CompositeAlphaFlagBitsKHR::ePostMultiplied,
		vk::CompositeAlphaFlagBitsKHR::eInherit,
	};
	for (uint32_t i = 0; i < sizeof(compositeAlphaFlags); i++) {
		if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
			compositeAlpha = compositeAlphaFlags[i];
			break;
		}
	}

	swapchainCreateInfo = vk::SwapchainCreateInfoKHR()
		.setSurface(surface)
		.setImageFormat(format)
		.setMinImageCount(desiredNumberOfSwapChainImages)
		.setPreTransform(preTransform)
		.setCompositeAlpha(compositeAlpha)
		.setImageArrayLayers(1)
		.setPresentMode(swapchainPresentMode)
		//.setOldSwapchain(VK_NULL_HANDLE);	
#ifndef __ANDROID__
		.setClipped(true)
#else
		.setClipped(false)
#endif
		.setImageColorSpace(vk::ColorSpaceKHR::eSrgbNonlinear)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr);
	swapchainCreateInfo.imageExtent.width = swapchainExtent.width;
	swapchainCreateInfo.imageExtent.height = swapchainExtent.height;

	if (graphicsFamilyIndex != presentFamilyIndex) {
		// If the graphics and present queues are from different queue families,
		// we either have to explicitly transfer ownership of images between the
		// queues, or we have to create the swapchain with imageSharingMode
		// as VK_SHARING_MODE_CONCURRENT
		uint32_t temp[2] = { (uint32_t)graphicsFamilyIndex, (uint32_t)presentFamilyIndex };
		swapchainCreateInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(temp);
	}

	swapchain = device.createSwapchainKHR(swapchainCreateInfo);
}

void HelloTriangleApplication::createImageviews() {
	swapchainImages = device.getSwapchainImagesKHR(swapchain);
	if (swapchainImages.size() < 1) {
		std::cout << "Failed to find supporting swapchain image" << std::endl;
		assert(0 && "Vulkan runtime error.");
	}

	for (uint32_t i = 0; i < swapchainImages.size(); i++) {
		swap_chain_buffer sc_buffer;

		vk::ImageViewCreateInfo color_image_view = vk::ImageViewCreateInfo()
			.setFormat(format)
			.setComponents(
				vk::ComponentMapping(
					vk::ComponentSwizzle::eR,
					vk::ComponentSwizzle::eG,
					vk::ComponentSwizzle::eB,
					vk::ComponentSwizzle::eA))
			.setSubresourceRange(
				vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1))
			.setViewType(vk::ImageViewType::e2D);

		sc_buffer.image = swapchainImages[i];
		color_image_view.setImage(sc_buffer.image);

		sc_buffer.view = device.createImageView(color_image_view);
		buffers.push_back(sc_buffer);
	}

	current_buffer = 0;
}

void HelloTriangleApplication::createDepthBuffer() {
	vk::ImageCreateInfo imageInfo;
	if (depth.format <= vk::Format::eUndefined) depth.format = vk::Format::eD16Unorm;

#ifdef __ANDROID__
	// Depth format needs to be VK_FORMAT_D24_UNORM_S8_UINT on Android.
	const vk::Format depthFormat = vk::Format::eD24UnormS8Uint;
#else
	const vk::Format depthFormat = depth.format;
#endif
	vk::FormatProperties props;
	props = gpu.getFormatProperties(depthFormat);
	if (props.linearTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
		imageInfo.setTiling(vk::ImageTiling::eLinear);
	}
	else if (props.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
		imageInfo.setTiling(vk::ImageTiling::eOptimal);
	}
	else {
		/* Try other depth formats? */
		std::cout << "depth_format Unsupported.\n";
		assert(0 && "Vulkan runtime error.");
	}

	imageInfo.setImageType(vk::ImageType::e2D)
		.setFormat(depthFormat)
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setUsage(vk::ImageUsageFlagBits::eDepthStencilAttachment);
	imageInfo.extent
		.setWidth(width)
		.setHeight(height);
		
	vk::MemoryAllocateInfo memAlloc = vk::MemoryAllocateInfo()
		.setAllocationSize(0)
		.setMemoryTypeIndex(0);

	vk::ImageViewCreateInfo viewInfo = vk::ImageViewCreateInfo()
		.setImage(nullptr)
		.setFormat(depthFormat)
		.setViewType(vk::ImageViewType::e2D);
	viewInfo.components
		.setR(vk::ComponentSwizzle::eR)
		.setG(vk::ComponentSwizzle::eG)
		.setB(vk::ComponentSwizzle::eB)
		.setA(vk::ComponentSwizzle::eA);
	viewInfo.subresourceRange
		.setAspectMask(vk::ImageAspectFlagBits::eDepth)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	if (depthFormat == vk::Format::eD16UnormS8Uint || depthFormat == vk::Format::eD24UnormS8Uint ||
		depthFormat == vk::Format::eD32SfloatS8Uint) {
		viewInfo.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
	}

	vk::MemoryRequirements memReqs;

	// Create image
	depth.image = device.createImage(imageInfo);	//여기서 에러남. why?

	memReqs = device.getImageMemoryRequirements(depth.image);

	memAlloc.setAllocationSize(memReqs.size);

	// Use the memory properties to determine the type of memory required
	memAlloc.setMemoryTypeIndex(
		getMemoryTypeFromProperties(memReqs.memoryTypeBits, 
			vk::MemoryPropertyFlagBits::eDeviceLocal)
	);

	// Allocate memory
	depth.mem = device.allocateMemory(memAlloc);

	// Bind memory
	device.bindImageMemory(depth.image, depth.mem, 0);

	// Create image view
	viewInfo.setImage(depth.image);
	depth.view = device.createImageView(viewInfo);
}

uint32_t HelloTriangleApplication::getMemoryTypeFromProperties(uint32_t typeBits, vk::MemoryPropertyFlags reqMask) {
	// Search memtypes to find first index with those properties
	for (uint32_t i = 0; i < gpuMemoryProps.memoryTypeCount; i++) {
		if ((typeBits & 1) == 1) {
			// Type is available, does it match user properties?
			if ((gpuMemoryProps.memoryTypes[i].propertyFlags & reqMask) == reqMask) {
				return i;
			}
		}
	}
	std::cout << "cannot find memory supporting properties\n";
	assert(0 && "Vulkan runtime error.");
	return UINT32_MAX;
}

void HelloTriangleApplication::createUniformBuffer() {
	float fov = glm::radians(45.0f);
	if (width > height) {
		fov *= static_cast<float>(height) / static_cast<float>(width);
	}
	Projection = glm::perspective(fov, (float)width / (float)height, 0.1f, 100.0f);
	View = glm::lookAt(glm::vec3(-5, 3, -10),  // Camera is at (-5,3,-10), in World Space
					   glm::vec3(0, 0, 0),     // and looks at the origin
					   glm::vec3(0, -1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
					   );
	Model = glm::mat4(1.0f);
	// Vulkan clip space has inverted Y and half Z.
	Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 
					0.0f, -1.0f, 0.0f, 0.0f, 
					0.0f, 0.0f, 0.5f, 0.0f, 
					0.0f, 0.0f, 0.5f, 1.0f);
	MVP = Clip * Projection * View * Model;

	vk::BufferCreateInfo bufInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eUniformBuffer)
		.setSize(sizeof(MVP))
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive);

	uniform_data.buf = device.createBuffer(bufInfo);

	vk::MemoryRequirements memReqs;
	memReqs = device.getBufferMemoryRequirements(uniform_data.buf);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setMemoryTypeIndex(0)
		.setAllocationSize(memReqs.size);

	allocInfo.memoryTypeIndex = getMemoryTypeFromProperties(memReqs.memoryTypeBits, (vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	uniform_data.mem = device.allocateMemory(allocInfo);

	uint8_t *pData;
	// c++ style로 바꿔야함...
	//pData = device.mapMemory(uniform_data.mem, 0, memReqs.size);
	device.mapMemory(uniform_data.mem, 0, memReqs.size, vk::MemoryMapFlags(), (void **)&pData);

	memcpy(pData, &MVP, sizeof(MVP));

	device.unmapMemory(uniform_data.mem);

	device.bindBufferMemory(uniform_data.buf, uniform_data.mem, 0);

	uniform_data.buffer_info.buffer = uniform_data.buf;
	uniform_data.buffer_info.offset = 0;
	uniform_data.buffer_info.range = sizeof(MVP);
}

void HelloTriangleApplication::createDescriptorPipelineLayouts(bool useTexture) {
	vk::DescriptorSetLayoutBinding layoutBindings[2];
	layoutBindings[0]
		.setBinding(0)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr);

	if (useTexture) {
		// using textures
		layoutBindings[1]
			.setBinding(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(nullptr);
	}

	// Next take layout bindings and use them to create a descriptor set layout
	vk::DescriptorSetLayoutCreateInfo  descriptorLayout = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(useTexture ? 2 : 1)
		.setPBindings(layoutBindings);

	//descLayout.resize(1);
	descLayout.push_back(device.createDescriptorSetLayout(descriptorLayout));
	//descLayout = device.createDescriptorSetLayout(descriptorLayout);
	
	// Now use the descriptor layout to create a pipeline layout
	vk::PipelineLayoutCreateInfo pPipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
		.setPushConstantRangeCount(0)
		.setPPushConstantRanges(nullptr)
		.setSetLayoutCount(1)
		.setPSetLayouts(descLayout.data());

	pipelineLayout = device.createPipelineLayout(pPipelineLayoutCreateInfo);
}

void HelloTriangleApplication::createShaders(const char *vertShaderText, const char *fragShaderText) {
	if (!(vertShaderText || fragShaderText)) return;

#ifndef __ANDROID__
	glslang::InitializeProcess();
#endif
	vk::ShaderModuleCreateInfo moduleCreateInfo;

	if (vertShaderText) {
		std::vector<unsigned int> vtx_spv;
		shaderStages[0]
			.setPSpecializationInfo(nullptr)
			.setStage(vk::ShaderStageFlagBits::eVertex)
			.setPName("main");

		bool res = GLSLtoSPV(vk::ShaderStageFlagBits::eVertex, vertShaderText, vtx_spv);
		assert(res);

		moduleCreateInfo
			.setCodeSize(vtx_spv.size() * sizeof(unsigned int))
			.setPCode(vtx_spv.data());

		shaderStages[0].module = device.createShaderModule(moduleCreateInfo);
	}

	if (fragShaderText) {
		std::vector<unsigned int> frag_spv;
		shaderStages[1]
			.setStage(vk::ShaderStageFlagBits::eFragment)
			.setPName("main");

		bool res = GLSLtoSPV(vk::ShaderStageFlagBits::eFragment, fragShaderText, frag_spv);
		assert(res);

		moduleCreateInfo
			.setCodeSize(frag_spv.size() * sizeof(unsigned int))
			.setPCode(frag_spv.data());

		shaderStages[1].module = device.createShaderModule(moduleCreateInfo);
	}

#ifndef __ANDROID__
	glslang::FinalizeProcess();
#endif
}

bool HelloTriangleApplication::GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, const char *pshader, std::vector<unsigned int> &spirv) {
	EShLanguage stage = FindLanguage(shaderType);
	glslang::TShader shader(stage);
	glslang::TProgram program;
	const char *shaderStrings[1];
	TBuiltInResource Resources;
	init_resources(Resources);

	// Enable SPIR-V and Vulkan rules when parsing GLSL
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	shaderStrings[0] = pshader;
	shader.setStrings(shaderStrings, 1);

	if (!shader.parse(&Resources, 100, false, messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		return false;  // something didn't work
	}

	program.addShader(&shader);

	//
	// Program-level processing...
	//

	if (!program.link(messages)) {
		puts(shader.getInfoLog());
		puts(shader.getInfoDebugLog());
		fflush(stdout);
		return false;
	}

	glslang::GlslangToSpv(*program.getIntermediate(stage), spirv);

	return true;
}

EShLanguage HelloTriangleApplication::FindLanguage(const vk::ShaderStageFlagBits shaderType) {
	switch (shaderType) {
	case vk::ShaderStageFlagBits::eVertex:
		return EShLangVertex;
	case vk::ShaderStageFlagBits::eTessellationControl:
		return EShLangTessControl;

	case vk::ShaderStageFlagBits::eTessellationEvaluation:
		return EShLangTessEvaluation;

	case vk::ShaderStageFlagBits::eGeometry:
		return EShLangGeometry;

	case vk::ShaderStageFlagBits::eFragment:
		return EShLangFragment;

	case vk::ShaderStageFlagBits::eCompute:
		return EShLangCompute;

	default:
		return EShLangVertex;
	}
}

void HelloTriangleApplication::init_resources(TBuiltInResource &Resources) {
	Resources.maxLights = 32;
	Resources.maxClipPlanes = 6;
	Resources.maxTextureUnits = 32;
	Resources.maxTextureCoords = 32;
	Resources.maxVertexAttribs = 64;
	Resources.maxVertexUniformComponents = 4096;
	Resources.maxVaryingFloats = 64;
	Resources.maxVertexTextureImageUnits = 32;
	Resources.maxCombinedTextureImageUnits = 80;
	Resources.maxTextureImageUnits = 32;
	Resources.maxFragmentUniformComponents = 4096;
	Resources.maxDrawBuffers = 32;
	Resources.maxVertexUniformVectors = 128;
	Resources.maxVaryingVectors = 8;
	Resources.maxFragmentUniformVectors = 16;
	Resources.maxVertexOutputVectors = 16;
	Resources.maxFragmentInputVectors = 15;
	Resources.minProgramTexelOffset = -8;
	Resources.maxProgramTexelOffset = 7;
	Resources.maxClipDistances = 8;
	Resources.maxComputeWorkGroupCountX = 65535;
	Resources.maxComputeWorkGroupCountY = 65535;
	Resources.maxComputeWorkGroupCountZ = 65535;
	Resources.maxComputeWorkGroupSizeX = 1024;
	Resources.maxComputeWorkGroupSizeY = 1024;
	Resources.maxComputeWorkGroupSizeZ = 64;
	Resources.maxComputeUniformComponents = 1024;
	Resources.maxComputeTextureImageUnits = 16;
	Resources.maxComputeImageUniforms = 8;
	Resources.maxComputeAtomicCounters = 8;
	Resources.maxComputeAtomicCounterBuffers = 1;
	Resources.maxVaryingComponents = 60;
	Resources.maxVertexOutputComponents = 64;
	Resources.maxGeometryInputComponents = 64;
	Resources.maxGeometryOutputComponents = 128;
	Resources.maxFragmentInputComponents = 128;
	Resources.maxImageUnits = 8;
	Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
	Resources.maxCombinedShaderOutputResources = 8;
	Resources.maxImageSamples = 0;
	Resources.maxVertexImageUniforms = 0;
	Resources.maxTessControlImageUniforms = 0;
	Resources.maxTessEvaluationImageUniforms = 0;
	Resources.maxGeometryImageUniforms = 0;
	Resources.maxFragmentImageUniforms = 8;
	Resources.maxCombinedImageUniforms = 8;
	Resources.maxGeometryTextureImageUnits = 16;
	Resources.maxGeometryOutputVertices = 256;
	Resources.maxGeometryTotalOutputComponents = 1024;
	Resources.maxGeometryUniformComponents = 1024;
	Resources.maxGeometryVaryingComponents = 64;
	Resources.maxTessControlInputComponents = 128;
	Resources.maxTessControlOutputComponents = 128;
	Resources.maxTessControlTextureImageUnits = 16;
	Resources.maxTessControlUniformComponents = 1024;
	Resources.maxTessControlTotalOutputComponents = 4096;
	Resources.maxTessEvaluationInputComponents = 128;
	Resources.maxTessEvaluationOutputComponents = 128;
	Resources.maxTessEvaluationTextureImageUnits = 16;
	Resources.maxTessEvaluationUniformComponents = 1024;
	Resources.maxTessPatchComponents = 120;
	Resources.maxPatchVertices = 32;
	Resources.maxTessGenLevel = 64;
	Resources.maxViewports = 16;
	Resources.maxVertexAtomicCounters = 0;
	Resources.maxTessControlAtomicCounters = 0;
	Resources.maxTessEvaluationAtomicCounters = 0;
	Resources.maxGeometryAtomicCounters = 0;
	Resources.maxFragmentAtomicCounters = 8;
	Resources.maxCombinedAtomicCounters = 8;
	Resources.maxAtomicCounterBindings = 1;
	Resources.maxVertexAtomicCounterBuffers = 0;
	Resources.maxTessControlAtomicCounterBuffers = 0;
	Resources.maxTessEvaluationAtomicCounterBuffers = 0;
	Resources.maxGeometryAtomicCounterBuffers = 0;
	Resources.maxFragmentAtomicCounterBuffers = 1;
	Resources.maxCombinedAtomicCounterBuffers = 1;
	Resources.maxAtomicCounterBufferSize = 16384;
	Resources.maxTransformFeedbackBuffers = 4;
	Resources.maxTransformFeedbackInterleavedComponents = 64;
	Resources.maxCullDistances = 8;
	Resources.maxCombinedClipAndCullDistances = 8;
	Resources.maxSamples = 4;
	Resources.limits.nonInductiveForLoops = 1;
	Resources.limits.whileLoops = 1;
	Resources.limits.doWhileLoops = 1;
	Resources.limits.generalUniformIndexing = 1;
	Resources.limits.generalAttributeMatrixVectorIndexing = 1;
	Resources.limits.generalVaryingIndexing = 1;
	Resources.limits.generalSamplerIndexing = 1;
	Resources.limits.generalVariableIndexing = 1;
	Resources.limits.generalConstantMatrixVectorIndexing = 1;
}

void HelloTriangleApplication::createDescriptorPool(bool useTexture) {
	// Depends on createUniformBuffer() and createDescriptorPipelineLayout()
	vk::DescriptorPoolSize typeCount[2];
	typeCount[0]
		.setType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1);
	if (useTexture) {
		typeCount[1]
			.setType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1);
	}

	vk::DescriptorPoolCreateInfo descirptorPool = vk::DescriptorPoolCreateInfo()
		.setMaxSets(1)
		.setPoolSizeCount(useTexture ? 2 : 1)
		.setPPoolSizes(typeCount);

	descPool = device.createDescriptorPool(descirptorPool);
}

void HelloTriangleApplication::createDescriptorSet(bool useTexture) {
	vk::DescriptorSetAllocateInfo allocInfo = vk::DescriptorSetAllocateInfo()
		.setDescriptorPool(descPool)
		.setDescriptorSetCount(1)
		.setPSetLayouts(descLayout.data());

	//descSet.resize(1);
	descSet = device.allocateDescriptorSets(allocInfo);
	//descSet.push_back(device.allocateDescriptorSets(allocInfo));
	//device.allocateDescriptorSets(allocInfo, descSet.data());

	vk::WriteDescriptorSet writes[2];
	writes[0]
		.setDstSet(descSet[0])
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setPBufferInfo(&uniform_data.buffer_info)
		.setDstArrayElement(0)
		.setDstBinding(0);

	if (useTexture) {
		writes[1]
			.setDstSet(descSet[0])
			.setDstBinding(1)
			.setDescriptorCount(1)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			//.setPImageInfo(texture_data.image_info)
			.setDstArrayElement(0);
	}

	device.updateDescriptorSets(useTexture ? 2 : 1, writes, 0, nullptr);


	
}

void HelloTriangleApplication::createRenderpass(bool include_depth, bool clear, vk::ImageLayout finalLayout) {
	// Depends on initSwapchain and initDepthbuffer
	vk::AttachmentDescription attachments[2];
	attachments[0]
		.setFormat(format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(finalLayout);

	if (include_depth) {
		attachments[1]
			.setFormat(format)
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(clear ? vk::AttachmentLoadOp::eClear : vk::AttachmentLoadOp::eDontCare)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eLoad)
			.setStencilStoreOp(vk::AttachmentStoreOp::eStore)
			.setInitialLayout(vk::ImageLayout::eUndefined)
			.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
	}

	vk::AttachmentReference colorReference = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference depthReference = vk::AttachmentReference()
		.setAttachment(1)
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setInputAttachmentCount(0)
		.setPInputAttachments(nullptr)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorReference)
		.setPResolveAttachments(nullptr)
		.setPDepthStencilAttachment(include_depth ? &depthReference : nullptr)
		.setPreserveAttachmentCount(0)
		.setPPreserveAttachments(nullptr);

	vk::RenderPassCreateInfo rpInfo = vk::RenderPassCreateInfo()
		.setAttachmentCount(include_depth ? 2 : 1)
		.setPAttachments(attachments)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(0)
		.setPDependencies(nullptr);

	renderPass = device.createRenderPass(rpInfo);
}

void HelloTriangleApplication::createFramebuffers(bool include_depth) {
	// Depends on createDepthbuffer, createRenderpass, initSwapchainExtension

	vk::ImageView attachments[2];
	attachments[1] = depth.view;

	vk::FramebufferCreateInfo fbInfo = vk::FramebufferCreateInfo()
		.setRenderPass(renderPass)
		.setAttachmentCount(include_depth ? 2 : 1)
		.setPAttachments(attachments)
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	uint32_t i;

	for (uint32_t i = 0; i < swapchainImages.size(); i++) {
		attachments[0] = buffers[i].view;
		framebuffers.push_back(device.createFramebuffer(fbInfo));
	}
}

void HelloTriangleApplication::createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture) {
	vk::BufferCreateInfo bufInfo = vk::BufferCreateInfo()
		.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
		.setSize(dataSize)
		.setQueueFamilyIndexCount(0)
		.setPQueueFamilyIndices(nullptr)
		.setSharingMode(vk::SharingMode::eExclusive);

	vertex_buffer.buf = device.createBuffer(bufInfo);

	vk::MemoryRequirements memReqs = device.getBufferMemoryRequirements(vertex_buffer.buf);

	vk::MemoryAllocateInfo allocInfo = vk::MemoryAllocateInfo()
		.setMemoryTypeIndex(0)
		.setAllocationSize(memReqs.size)
		.setMemoryTypeIndex(getMemoryTypeFromProperties(memReqs.memoryTypeBits,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	
	vertex_buffer.mem = device.allocateMemory(allocInfo);

	uint8_t *pData;
	device.mapMemory(vertex_buffer.mem, 0, memReqs.size, vk::MemoryMapFlags(), (void **)&pData);

	memcpy(pData, vertexData, dataSize);

	device.unmapMemory(vertex_buffer.mem);

	device.bindBufferMemory(vertex_buffer.buf, vertex_buffer.mem, 0);

	vi_binding
		.setBinding(0)
		.setInputRate(vk::VertexInputRate::eVertex)
		.setStride(dataStride);

	vi_attribs[0]
		.setBinding(0)
		.setLocation(0)
		.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setOffset(0);

	vi_attribs[1]
		.setBinding(0)
		.setLocation(1)
		.setFormat(useTexture ? vk::Format::eR32G32Sfloat : vk::Format::eR32G32B32A32Sfloat)
		.setOffset(16);		//WHY 16?
	
}

void HelloTriangleApplication::createPipelineCache() {
	vk::PipelineCacheCreateInfo cacheInfo = vk::PipelineCacheCreateInfo()
		.setInitialDataSize(0)
		.setPInitialData(nullptr);

	pipelineCache = device.createPipelineCache(cacheInfo);
}

void HelloTriangleApplication::createPipeline(vk::Bool32 include_depth, vk::Bool32 include_vi) {
	//VK_DYNAMIC_STATE_RANGE_SIZE 에 대한 c++ 바인딩이 따로 없는듯, 일단 enum에서 대충 값 찾아서 넣음
	vk::DynamicState dynamicStateEnables[9];
	memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
	vk::PipelineDynamicStateCreateInfo dynamicInfo = vk::PipelineDynamicStateCreateInfo()
		.setPDynamicStates(dynamicStateEnables)
		.setDynamicStateCount(0);

	vk::PipelineVertexInputStateCreateInfo vi;
	memset(&vi, 0, sizeof(vi));
	if (include_vi) {
		vi.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&vi_binding)
			.setVertexAttributeDescriptionCount(2)
			.setPVertexAttributeDescriptions(vi_attribs);
	}

	vk::PipelineInputAssemblyStateCreateInfo ia = vk::PipelineInputAssemblyStateCreateInfo()
		.setPrimitiveRestartEnable(VK_FALSE)
		.setTopology(vk::PrimitiveTopology::eTriangleList);

	vk::PipelineRasterizationStateCreateInfo rs = vk::PipelineRasterizationStateCreateInfo()
		.setPolygonMode(vk::PolygonMode::eFill)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0)
		.setDepthBiasClamp(0)
		.setDepthBiasSlopeFactor(0)
		.setLineWidth(1.0f);

	

	vk::PipelineColorBlendAttachmentState att_state[1];
	att_state[0]
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
			vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE)
		.setAlphaBlendOp(vk::BlendOp::eAdd)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcColorBlendFactor(vk::BlendFactor::eZero)
		.setDstColorBlendFactor(vk::BlendFactor::eZero)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eZero)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero);
	
	std::array<float, 4> constants = { 1.0f, 1.0f, 1.0f, 1.0f };
	vk::PipelineColorBlendStateCreateInfo cb = vk::PipelineColorBlendStateCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(att_state)
		.setLogicOpEnable(VK_FALSE)
		.setLogicOp(vk::LogicOp::eNoOp)
		.setBlendConstants(constants);

	vk::PipelineViewportStateCreateInfo vp = vk::PipelineViewportStateCreateInfo()
#ifndef __ANDROID
		.setViewportCount(1)
		.setScissorCount(1)
		.setPScissors(nullptr)
		.setPViewports(nullptr);
	// 순서가 중요한지?
	dynamicStateEnables[dynamicInfo.dynamicStateCount++] = vk::DynamicState::eViewport;
	dynamicStateEnables[dynamicInfo.dynamicStateCount++] = vk::DynamicState::eScissor;
#else
		// Temporary disabling dynamic viewport on Android because some of drivers doesn't
		// support the feature.
		;
	vk::Viewport viewports = vk::Viewport()
		.setMinDepth(0.0f)
		.setMaxDepth(1.0f)
		.setX(0)
		.setY(0)
		.setWidth(width)
		.setHeight(height);

	vk::Rect2D scissor = vk::Rect2D();
	scissor.extent
		.setWidth(width)
		.setHeight(height);
	scissor.offset
		.setX(0)
		.setY(0);

	vp.setViewportCount(1)
		.setScissorCount(1)
		.setPScissors(&scissor)
		.setPViewports(&viewports);
#endif

	vk::PipelineDepthStencilStateCreateInfo ds = vk::PipelineDepthStencilStateCreateInfo()
		.setDepthTestEnable(include_depth)
		.setDepthWriteEnable(include_depth)
		.setDepthCompareOp(vk::CompareOp::eLessOrEqual)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setStencilTestEnable(VK_FALSE);
	ds.back
		.setFailOp(vk::StencilOp::eKeep)
		.setPassOp(vk::StencilOp::eKeep)
		.setCompareOp(vk::CompareOp::eAlways)
		.setCompareMask(0)
		.setReference(0)
		.setDepthFailOp(vk::StencilOp::eKeep)
		.setWriteMask(0);
	ds.setMinDepthBounds(0)
		.setMaxDepthBounds(0)
		.setStencilTestEnable(VK_FALSE)
		.setFront(ds.back);

	vk::PipelineMultisampleStateCreateInfo ms = vk::PipelineMultisampleStateCreateInfo()
		.setPSampleMask(nullptr)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setSampleShadingEnable(VK_FALSE)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE)
		.setMinSampleShading(0.0);

	vk::GraphicsPipelineCreateInfo plInfo = vk::GraphicsPipelineCreateInfo()
		.setLayout(pipelineLayout)
		.setBasePipelineHandle(nullptr)
		.setBasePipelineIndex(0)
		.setPVertexInputState(&vi)
		.setPInputAssemblyState(&ia)
		.setPRasterizationState(&rs)
		.setPColorBlendState(&cb)
		.setPTessellationState(nullptr)
		.setPMultisampleState(&ms)
		.setPDynamicState(&dynamicInfo)
		.setPViewportState(&vp)
		.setPDepthStencilState(&ds)
		.setPStages(shaderStages)
		.setStageCount(2)
		.setRenderPass(renderPass)
		.setSubpass(0);

	pipeline = device.createGraphicsPipeline(pipelineCache, plInfo);

}

void HelloTriangleApplication::recordCommands() {
	vk::ClearValue clear_values[2];
	clear_values[0].color.float32[0] = 0.2f;
	clear_values[0].color.float32[1] = 0.2f;
	clear_values[0].color.float32[2] = 0.2f;
	clear_values[0].color.float32[3] = 0.2f;
	clear_values[1].depthStencil.depth = 1.0f;
	clear_values[1].depthStencil.stencil = 0;

	vk::SemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo = vk::SemaphoreCreateInfo();
	vk::Semaphore imageAcquiredSemaphore = device.createSemaphore(imageAcquiredSemaphoreCreateInfo);

	// Get the index of the next available swapchain image:
	current_buffer = device.acquireNextImageKHR(swapchain, UINT64_MAX, imageAcquiredSemaphore, nullptr);

}

void HelloTriangleApplication::destroyInstance() {
	
}

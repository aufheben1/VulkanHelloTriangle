#include "HelloTriangleApplication.h"

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

	addDeviceExtansions();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
		
	createSwapchain();
	//createImageviews();

	//createRenderPass();
	//createFramebuffers();

	createCommandBuffer();
	////createDescriptorSetLayout();
	//createGraphicsPipeline();

	////createVertexBuffer();
	////createIndexBuffer();

	////createStagingUniformBuffer();
	////createUniformBuffer();
	////createDescriptorPool();
	////createDescriptorSet();

	//recordCommandBuffers();
	//createSemaphores();
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

void HelloTriangleApplication::addDeviceExtansions() {
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

	for (unsigned int i = 0; i < queueProps.size(); i++) {
		if (queueProps[i].queueFlags & vk::QueueFlagBits::eGraphics) {
			graphicsFamilyIndex = i;
			return true;
		}
	}

	return false;
}

void HelloTriangleApplication::createLogicalDevice() {

	float queue_priorities[1] = { 0.0 };
	queueInfo = vk::DeviceQueueCreateInfo()
		.setQueueCount(1)
		.setPQueuePriorities(queue_priorities);

	deviceInfo = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queueInfo);

	try {
		device = gpu.createDevice(deviceInfo);
	}
	catch (const std::exception& e) {
		std::cout << "Could not create a logical device: " << e.what() << std::endl;
		assert(0 && "Vulkan runtime error.");
	}
}

void HelloTriangleApplication::createCommandBuffer() {
	// Create a command pool to allocate our command buffer from
	cmdPoolInfo = vk::CommandPoolCreateInfo()
		.setQueueFamilyIndex(graphicsFamilyIndex);

	cmdPool = device.createCommandPool(cmdPoolInfo);

	// Create the command buffer from the command pool
	cmdInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(cmdPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	cmd = device.allocateCommandBuffers(cmdInfo);
}

void HelloTriangleApplication::createSwapchain() {
	// Check our queue supports presenting
	vk::Bool32 supportPresent = gpu.getSurfaceSupportKHR(graphicsFamilyIndex, surface);
	bool found = false;
	if (!supportPresent) {
		// If not, iterate over each queue to find one support presenting
		for (int i = 0; i < queueProps.size(); i++) {
			if (gpu.getSurfaceSupportKHR(i, surface)) {
				presentFamilyIndex = i;
				found = true;
				break;
			}
		}
	}
	else {
		presentFamilyIndex = graphicsFamilyIndex;
		found = true;
	}

	if (!found) {
		std::cout << "Failed to find a GPU supporting present queue!" << std::endl;
		assert(0 && "Vulkan runtime error.");
	}

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

	// Get surface capabilities
	surfCapabilities = gpu.getSurfaceCapabilitiesKHR(surface);

	// ???
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
	// We need to acquire only 1 presentable image at at time.
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
		.setImageExtent(swapchainExtent)	//width, height 따로 해야하나?
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

	swapchain = device.createSwapchainKHR(swapchainCreateInfo, nullptr);

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

void HelloTriangleApplication::destroyInstance() {
	
}

//deviceExtensions.push_back("VK_KHR_sqapchain")
#include "HelloTriangleApplication.h"

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
	getAvailableWSIExtensions();

	createInstance();
	createSurface();
	selectPhysicalDevice();
	createLogicalDevice();
	createCommandBuffer();
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

void HelloTriangleApplication::getAvailableWSIExtensions()
{
	// Use validation layers if this is a debug build, and use WSI extensions regardless
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif
	extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	extensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_MIR_KHR)
	extensions.push_back(VK_KHR_MIR_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	extensions.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_XLIB_KHR)
	extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
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
		.setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
		.setPpEnabledExtensionNames(extensions.data())
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

void HelloTriangleApplication::createSurface() {
	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		assert(0 && "SDL error.");
	}
	window = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
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
			queueFamilyIndex = i;
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
		.setQueueFamilyIndex(queueFamilyIndex);

	cmdPool = device.createCommandPool(cmdPoolInfo);

	// Create the command buffer from the command pool
	cmdInfo = vk::CommandBufferAllocateInfo()
		.setCommandPool(cmdPool)
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1);

	cmd = device.allocateCommandBuffers(cmdInfo);
}

void HelloTriangleApplication::createSwapChain() {
	// Construct the surface description:
}

void HelloTriangleApplication::destroyInstance() {
	
}

//deviceExtensions.push_back("VK_KHR_sqapchain")
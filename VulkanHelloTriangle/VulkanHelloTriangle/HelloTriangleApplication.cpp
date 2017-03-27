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
	createInstance();
	selectPhysicalDevice();
	createLogicalDevice();
}

void HelloTriangleApplication::mainLoop() {

}

void HelloTriangleApplication::createInstance() {
	// Use validation layers if this is a debug build, and use WSI extensions regardless
	extensions = getAvailableWSIExtensions();
#if defined(_DEBUG)
	layers.push_back("VK_LAYER_LUNARG_standard_validation");
#endif

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

void HelloTriangleApplication::selectPhysicalDevice() {
	// enumerate physical devices (GPUs)
	std::vector<vk::PhysicalDevice> gpuList = instance.enumeratePhysicalDevices();
	
	bool found = false;
	for (const auto& device : gpuList) {
		if (isDeviceSuitable(device)) {
			gpu = device;
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

void HelloTriangleApplication::destroyInstance() {
	
}


vk::SurfaceKHR HelloTriangleApplication::createVulkanSurface(const vk::Instance& instance, SDL_Window* window)
{
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
		return instance.createAndroidSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_MIR) && defined(VK_USE_PLATFORM_MIR_KHR)
	case SDL_SYSWM_MIR: {
		vk::MirSurfaceCreateInfoKHR surfaceInfo = vk::MirSurfaceCreateInfoKHR()
			.setConnection(windowInfo.info.mir.connection)
			.setMirSurface(windowInfo.info.mir.surface);
		return instance.createMirSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WAYLAND) && defined(VK_USE_PLATFORM_WAYLAND_KHR)
	case SDL_SYSWM_WAYLAND: {
		vk::WaylandSurfaceCreateInfoKHR surfaceInfo = vk::WaylandSurfaceCreateInfoKHR()
			.setDisplay(windowInfo.info.wl.display)
			.setSurface(windowInfo.info.wl.surface);
		return instance.createWaylandSurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_WINDOWS) && defined(VK_USE_PLATFORM_WIN32_KHR)
	case SDL_SYSWM_WINDOWS: {
		vk::Win32SurfaceCreateInfoKHR surfaceInfo = vk::Win32SurfaceCreateInfoKHR()
			.setHinstance(GetModuleHandle(NULL))
			.setHwnd(windowInfo.info.win.window);
		return instance.createWin32SurfaceKHR(surfaceInfo);
	}
#endif

#if defined(SDL_VIDEO_DRIVER_X11) && defined(VK_USE_PLATFORM_XLIB_KHR)
	case SDL_SYSWM_X11: {
		vk::XlibSurfaceCreateInfoKHR surfaceInfo = vk::XlibSurfaceCreateInfoKHR()
			.setDpy(windowInfo.info.x11.display)
			.setWindow(windowInfo.info.x11.window);
		return instance.createXlibSurfaceKHR(surfaceInfo);
	}
#endif

	default:
		throw std::system_error(std::error_code(), "Unsupported window manager is in use.");
	}
}

std::vector<const char*> HelloTriangleApplication::getAvailableWSIExtensions()
{
	std::vector<const char*> extensions;
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

	return extensions;
}

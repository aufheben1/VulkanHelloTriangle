#ifndef _VULKAN_HELLO_TRIANGLE_APPLICATION_H_
#define _VULKAN_HELLO_TRIANGLE_APPLICATION_H_

// Enable the WSI extensions
#if defined(__ANDROID__)
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <vulkan/vulkan.hpp>

#include <iostream>
#include <vector>

class HelloTriangleApplication {
public:
	void run();
	void terminate();

private:
	void initVulkan();
	void mainLoop();
	
	void getAvailableWSIExtensions();
	void createInstance();
	void createSurface();
	void selectPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	void createLogicalDevice();
	void createCommandBuffer();
	void createSwapChain();
		
	void destroyInstance();

public:

private:
	vk::SurfaceKHR surface;
	SDL_Window* window;

	vk::Instance instance;
	std::vector<const char*> extensions;
	std::vector<const char*> layers;

	vk::ApplicationInfo appInfo;
	vk::InstanceCreateInfo instInfo;

	vk::PhysicalDevice gpu;
	vk::PhysicalDeviceProperties gpuProps = {};
	vk::PhysicalDeviceFeatures gpuFeatures = {};
	vk::PhysicalDeviceMemoryProperties gpuMemoryProps = {};
	std::vector<vk::QueueFamilyProperties> queueProps;

	vk::DeviceQueueCreateInfo queueInfo;
	uint32_t queueFamilyIndex;
	vk::DeviceCreateInfo deviceInfo;
	vk::Device device;

	vk::CommandPoolCreateInfo cmdPoolInfo;
	vk::CommandPool cmdPool;
	vk::CommandBufferAllocateInfo cmdInfo;
	std::vector<vk::CommandBuffer> cmd;

};


#endif
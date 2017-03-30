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
private:
	/*
	* Keep each of our swap chain buffers' image, command buffer and view in one
	* spot
	*/
	typedef struct _swap_chain_buffers {
		VkImage image;
		VkImageView view;
	} swap_chain_buffer;


public:
	HelloTriangleApplication();

	void run();
	void terminate();

private:
	void initVulkan();
	void mainLoop();
	
	void addInstanceExtensions();
	void createInstance();

	void addDeviceExtensions();
	void createSurface();
	void selectPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	void createLogicalDevice();
	void createCommandPool();
	void createCommandBuffer();
	void createSwapchain();
	void initSwapchainExtension();
	void createImageviews();
		
	void destroyInstance();

public:

private:
	vk::SurfaceKHR surface;
	SDL_Window* window;
	int width, height;

	vk::Instance instance;
	std::vector<const char*> instanceExtensions;
	std::vector<const char*> deviceExtensions;
	std::vector<const char*> layers;

	vk::ApplicationInfo appInfo;
	vk::InstanceCreateInfo instInfo;

	vk::PhysicalDevice gpu;
	vk::PhysicalDeviceProperties gpuProps = {};
	vk::PhysicalDeviceFeatures gpuFeatures = {};
	vk::PhysicalDeviceMemoryProperties gpuMemoryProps = {};
	std::vector<vk::QueueFamilyProperties> queueProps;

	vk::DeviceQueueCreateInfo queueInfo;
	uint32_t graphicsFamilyIndex;
	uint32_t presentFamilyIndex;
	vk::DeviceCreateInfo deviceInfo;
	vk::Device device;

	vk::CommandPoolCreateInfo cmdPoolInfo;
	vk::CommandPool cmdPool;
	vk::CommandBufferAllocateInfo cmdInfo;
	std::vector<vk::CommandBuffer> cmd;

	vk::SwapchainKHR swapchain;
	vk::SwapchainCreateInfoKHR swapchainCreateInfo;
	vk::Format format;
	vk::SurfaceCapabilitiesKHR surfCapabilities;

	std::vector<vk::Image> swapchainImages;
	std::vector<swap_chain_buffer> buffers;
	uint32_t current_buffer;
};


#endif
#ifndef _VULKAN_HELLO_TRIANGLE_APPLICATION_H_
#define _VULKAN_HELLO_TRIANGLE_APPLICATION_H_

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
	
	void createInstance();
	void selectPhysicalDevice();
	bool isDeviceSuitable(vk::PhysicalDevice device);
	void createLogicalDevice();
	
	
	
	void destroyInstance();

public:

private:
	vk::Instance instance = VK_NULL_HANDLE;
	std::vector<const char*> extensions;
	std::vector<const char*> layers;

	vk::ApplicationInfo appInfo;
	vk::InstanceCreateInfo instInfo;

	vk::PhysicalDevice gpu = VK_NULL_HANDLE;
	vk::PhysicalDeviceProperties gpuProps = {};
	vk::PhysicalDeviceFeatures gpuFeatures = {};
	vk::PhysicalDeviceMemoryProperties gpuMemoryProps = {};
	std::vector<vk::QueueFamilyProperties> queueProps;

	vk::DeviceQueueCreateInfo queueInfo;
	uint32_t queueFamilyIndex;
	vk::DeviceCreateInfo deviceInfo;
	vk::Device device;

	vk::SurfaceKHR surface;
	SDL_Window* window;

	vk::SurfaceKHR createVulkanSurface(const vk::Instance& instance, SDL_Window* window);
	std::vector<const char*> getAvailableWSIExtensions();
};


#endif
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
#include <SPIRV\GlslangToSpv.h>

class HelloTriangleApplication {
private:
	/*
	* Keep each of our swap chain buffers' image, command buffer and view in one
	* spot
	*/
	typedef struct _swap_chain_buffers {
		vk::Image image;
		vk::ImageView view;
	} swap_chain_buffer;

	struct {
		vk::Format format;

		vk::Image image;
		vk::DeviceMemory mem;
		vk::ImageView view;
	} depth;

	struct {
		vk::Buffer buf;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo buffer_info;
	} uniform_data;

	struct {
		vk::Buffer buf;
		vk::DeviceMemory mem;
		vk::DescriptorBufferInfo buffer_info;
	} vertex_buffer;
	vk::VertexInputBindingDescription vi_binding;
	vk::VertexInputAttributeDescription vi_attribs[2];

	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 Clip;
	glm::mat4 MVP;

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
	void beginCommandBuffer();
	void createDeviceQueue();
	void createSwapchain();
	void initSwapchainExtension();
	void createImageviews();
		
	void createDepthBuffer();
	uint32_t getMemoryTypeFromProperties(uint32_t typeBits, vk::MemoryPropertyFlags reqMask);
	
	void createUniformBuffer();
	void createDescriptorPipelineLayouts(bool useTexture);
	
	void createShaders(const char *vertShaderText, const char *fragShaderText);
	bool GLSLtoSPV(const vk::ShaderStageFlagBits shaderType, const char *pshader, std::vector<unsigned int> &spirv);
	EShLanguage FindLanguage(const vk::ShaderStageFlagBits shaderType);
	void init_resources(TBuiltInResource &Resources);
	
	
	void createDescriptorPool(bool useTexture);
	void createDescriptorSet(bool useTexture);

	void createRenderpass(bool include_depth, bool clear = true, vk::ImageLayout finalLayout = vk::ImageLayout::ePresentSrcKHR);
	void createFramebuffers(bool include_depth);
	void createVertexBuffer(const void* vertexData, uint32_t dataSize, uint32_t dataStride, bool useTexture);

	void createPipelineCache();
	void createPipeline(vk::Bool32 include_depth, vk::Bool32 include_vi = true);

	void recordCommands();
	void initViewports();
	void initScissors();


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
	vk::Queue graphicsQueue;
	vk::Queue presentQueue;

	vk::DeviceCreateInfo deviceInfo;
	vk::Device device;

	vk::CommandPoolCreateInfo cmdPoolInfo;
	vk::CommandPool cmdPool;
	vk::CommandBufferAllocateInfo cmdInfo;
	std::vector<vk::CommandBuffer> cmd;
	vk::CommandBufferBeginInfo cmdBufInfo;

	vk::SwapchainKHR swapchain;
	vk::SwapchainCreateInfoKHR swapchainCreateInfo;
	vk::Format format;
	vk::SurfaceCapabilitiesKHR surfCapabilities;

	std::vector<vk::Image> swapchainImages;
	std::vector<swap_chain_buffer> buffers;
	uint32_t current_buffer;

	std::vector<vk::DescriptorSetLayout> descLayout;
	//vk::DescriptorSetLayout descLayout;
	vk::PipelineLayout pipelineLayout;

	vk::PipelineShaderStageCreateInfo shaderStages[2];

	vk::DescriptorPool descPool;
	std::vector<vk::DescriptorSet> descSet;
	
	vk::RenderPass renderPass;
	std::vector<vk::Framebuffer> framebuffers;

	vk::PipelineCache pipelineCache;
	vk::Pipeline pipeline;

	vk::Viewport viewport;
	vk::Rect2D scissor;
};


#endif
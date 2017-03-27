#ifndef _VULKAN_HELLO_TRIANGLE_APPLICATION_H_
#define _VULKAN_HELLO_TRIANGLE_APPLICATION_H_

class HelloTriangleApplication {
public:
	void run();

private:
	void initVulkan();
	void mainLoop();
};


#endif
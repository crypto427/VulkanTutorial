#include <stdio.h>

#include "Include/vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 600

typedef struct HelloTriangleApplication {
    GLFWwindow *window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
} HelloTriangleApplication;

void run(HelloTriangleApplication* app);
GLFWwindow* initWindow();
void initVulkan(VkInstance* instance);
void mainLoop(GLFWwindow* window);
void cleanup(GLFWwindow* window, VkInstance* instance);

void createInstance(VkInstance* instance);

int main() {
    HelloTriangleApplication app;
    run(&app);
    return 0;
}

void run(HelloTriangleApplication* app)
{
    app->window = initWindow();
    if(!app->window)
    {
        fprintf(stderr, "Failed to create window\n");
        exit(1);
    }
    initVulkan(&app->instance);
    mainLoop(app->window);
    cleanup(app->window, &app->instance);
}

GLFWwindow* initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
}

void initVulkan(VkInstance* instance)
{
    createInstance(instance);
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup(GLFWwindow* window, VkInstance* instance)
{
    vkDestroyInstance(*instance, NULL);

    glfwDestroyWindow(window);

    glfwTerminate();
}

void createInstance(VkInstance* instance)
{
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Hello Triangle",
        .pNext = NULL,
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName      = "No Engine",
        .engineVersion    = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion       = VK_API_VERSION_1_4
    };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);


    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensionProperties = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties);
    
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        int found = 0;
        printf("%2d: %s\n", i, glfwExtensions[i]);
        for (uint32_t j = 0; j < extensionCount; j++) {
            if(strcmp(extensionProperties[j].extensionName, glfwExtensions[i]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Required GLFW extension not supported: %s\n", glfwExtensions[i]);
            free(extensionProperties);
            exit(1);
        }
    }

    free(extensionProperties);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .pApplicationInfo        = &appInfo,
        .enabledExtensionCount   = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions
    };

    VkResult result = vkCreateInstance(&createInfo, NULL, instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance failed: %d\n", result);
        exit(1);
    }
}   

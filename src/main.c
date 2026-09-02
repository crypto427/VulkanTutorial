#include <stdio.h>

#include "Include/vulkan/vulkan.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 600


const char* validationLayers[] = {
    "VK_LAYER_KHRONOS_validation"
};
const uint32_t numValidationLayers = 1;

#ifdef NDEBUG
#define ENABLE_VALIDATION_LAYERS 0
#else
#define ENABLE_VALIDATION_LAYERS 1
#endif


typedef struct HelloTriangleApplication {
    GLFWwindow *window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkDebugUtilsMessengerEXT debugMessenger;
} HelloTriangleApplication;

void run(HelloTriangleApplication* app);
GLFWwindow* initWindow();
void initVulkan(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger);
void mainLoop(GLFWwindow* window);
void cleanup(HelloTriangleApplication* app);


void createInstance(VkInstance* instance);


const char** getRequiredInstanceExtensions(uint32_t *out_total);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
static const char* severityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);


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
    initVulkan(&app->instance, &app->debugMessenger);
    mainLoop(app->window);
    cleanup(app);
}

GLFWwindow* initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
}

void initVulkan(VkInstance* instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
    createInstance(instance);
    setupDebugMessenger(*instance, debugMessenger);
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup(HelloTriangleApplication* app)
{
    if (ENABLE_VALIDATION_LAYERS) {
        PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
            (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(app->instance, "vkDestroyDebugUtilsMessengerEXT");
        vkDestroyDebugUtilsMessengerEXT(app->instance, app->debugMessenger, NULL);
    }
    vkDestroyInstance(app->instance, NULL);

    glfwDestroyWindow(app->window);

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
    
    uint32_t layerPropertiesCount = 0;
    VkLayerProperties *layerProperties = NULL;

    if(ENABLE_VALIDATION_LAYERS) {
        layerPropertiesCount = 0;
        vkEnumerateInstanceLayerProperties(&layerPropertiesCount, NULL);

        layerProperties = malloc(layerPropertiesCount * sizeof(VkLayerProperties)) ;
        vkEnumerateInstanceLayerProperties(&layerPropertiesCount, layerProperties);

        for (uint32_t i = 0; i < numValidationLayers; i++) {
            int found = 0;
            printf("%2d: %s\n", i, validationLayers[i]);
            for (uint32_t j = 0; j < layerPropertiesCount; j++) {
                if(strcmp(layerProperties[j].layerName, validationLayers[i]) == 0) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                fprintf(stderr, "Required layer not supported: %s\n", validationLayers[i]);
                free(layerProperties);
                exit(1);
            }
        }
    }
    

    uint32_t requiredExtensionsCount = 0;
    const char **requiredExtensions = getRequiredInstanceExtensions(&requiredExtensionsCount);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

    VkExtensionProperties *extensionProperties = malloc(extensionCount * sizeof(VkExtensionProperties));
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties);
    
    for (uint32_t i = 0; i < requiredExtensionsCount; i++) {
        int found = 0;
        printf("%2d: %s\n", i, requiredExtensions[i]);
        for (uint32_t j = 0; j < extensionCount; j++) {
            if(strcmp(extensionProperties[j].extensionName, requiredExtensions[i]) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(stderr, "Required GLFW extension not supported: %s\n", requiredExtensions[i]);
            free(extensionProperties);
            exit(1);
        }
    }

    free(extensionProperties);
    
    
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        // .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        .enabledLayerCount = numValidationLayers,
        .ppEnabledLayerNames = validationLayers,
        .pApplicationInfo        = &appInfo,
        .enabledExtensionCount   = requiredExtensionsCount,
        .ppEnabledExtensionNames = requiredExtensions
    };

    VkResult result = vkCreateInstance(&createInfo, NULL, instance);
    if (result != VK_SUCCESS) {
        fprintf(stderr, "vkCreateInstance failed: %d\n", result);
        exit(1);
    }

    free(requiredExtensions);
    if (ENABLE_VALIDATION_LAYERS)
        if (layerProperties)
            free(layerProperties);
}


const char** getRequiredInstanceExtensions(uint32_t *out_total) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    uint32_t total = glfwExtensionCount + ENABLE_VALIDATION_LAYERS;
    
    const char** extensions = malloc(total * sizeof(const char *));
    if (!extensions) return NULL;

    for (uint32_t i = 0; i < glfwExtensionCount; i++)
        extensions[i] = glfwExtensions[i];

    if (ENABLE_VALIDATION_LAYERS)
        extensions[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    *out_total = total;
    return extensions;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    (void)pUserData; // to suppress unused param warnings
    fprintf(stderr, "validation layer [%s] type=%u: %s\n",
            severityToString(severity), (unsigned)type, pCallbackData->pMessage);
    return VK_FALSE;
}

static const char* severityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "verbose";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return "info";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "warning";
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return "error";
        default: return "unknown";
    }
}

void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger)
{
    if(!ENABLE_VALIDATION_LAYERS) return;
    VkDebugUtilsMessageSeverityFlagsEXT severityFlags = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;

    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT =  {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = severityFlags,
        .messageType = messageTypeFlags,
        .pfnUserCallback = &debugCallback
    };

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
    (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

    vkCreateDebugUtilsMessengerEXT(instance, 
        &debugUtilsMessengerCreateInfoEXT, 
        NULL, 
        debugMessenger);
}

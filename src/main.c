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
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkPhysicalDeviceFeatures2 deviceFeatures;
    VkQueue graphicsQueue;
} HelloTriangleApplication;

void run(HelloTriangleApplication* app);
GLFWwindow* initWindow();
void initVulkan(HelloTriangleApplication* app);
void mainLoop(GLFWwindow* window);
void cleanup(HelloTriangleApplication* app);


void createInstance(VkInstance* instance);


const char** getRequiredInstanceExtensions(uint32_t *out_total);
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT severity, VkDebugUtilsMessageTypeFlagsEXT type, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData);
static const char* severityToString(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
void setupDebugMessenger(VkInstance instance, VkDebugUtilsMessengerEXT* debugMessenger);


void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice);


void createLogicalDevice(HelloTriangleApplication *app);


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
    initVulkan(app);
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

void initVulkan(HelloTriangleApplication* app)
{
    createInstance(&app->instance);
    setupDebugMessenger(app->instance, &app->debugMessenger);
    pickPhysicalDevice(app->instance, &app->physicalDevice);
    createLogicalDevice(app);
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

    vkDestroyDevice(app->device, NULL);

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


void pickPhysicalDevice(VkInstance instance, VkPhysicalDevice* physicalDevice)
{
    uint32_t physicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
    if(physicalDeviceCount == 0) {
        fprintf(stderr, "Failed to find GPUs with Vulkan support!\n");
        exit(1);
    }
    VkPhysicalDevice *physicalDevices = malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
    if(!physicalDevices) {
        fprintf(stderr, "Failed to allocate memory to physicalDevices array!\n");
        exit(1);
    } 
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

    VkPhysicalDeviceProperties2 *physicalDeviceProperties = malloc(sizeof(VkPhysicalDeviceProperties2));
    if(!physicalDeviceProperties) {
        fprintf(stderr, "Failed to malloc *physicalDeviceProperties\n");
        exit(1);
    }

    physicalDeviceProperties->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    physicalDeviceProperties->pNext = NULL;

    VkPhysicalDeviceFeatures2 *pFeatures = malloc(sizeof(VkPhysicalDeviceFeatures2));
    if(!pFeatures) {
        fprintf(stderr, "Failed to malloc *pFeatures\n");
        exit(1);
    }

    pFeatures->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    pFeatures->pNext = NULL;

    int selectedDeviceIndex = -1, highest_score = 0;
    for(uint32_t i = 0; i < physicalDeviceCount; i++){
        int score = 0;
        vkGetPhysicalDeviceProperties2(physicalDevices[i], physicalDeviceProperties);
        fprintf(stdout, "GPU: %s\n", physicalDeviceProperties->properties.deviceName);
        if(!(physicalDeviceProperties->properties.apiVersion >= VK_API_VERSION_1_3)) {
            fprintf(stdout, "GPU api version not >= 1.3 \n");
            continue;
        }

        uint32_t pQueueFamilyCount;
        
        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevices[i], &pQueueFamilyCount, NULL);

        VkQueueFamilyProperties2 *pQueueFamilyProperties = malloc(pQueueFamilyCount * sizeof(VkQueueFamilyProperties2));

        for(uint32_t j = 0; j < pQueueFamilyCount; j++) {
            pQueueFamilyProperties[j].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
            pQueueFamilyProperties[j].pNext = NULL;
        }

        vkGetPhysicalDeviceQueueFamilyProperties2(physicalDevices[i], &pQueueFamilyCount, pQueueFamilyProperties);

        int supportsGraphics = 0;
        for(uint32_t j = 0; j < pQueueFamilyCount; j++) {
            if (pQueueFamilyProperties[j].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    supportsGraphics = 1;
                    fprintf(stdout, "Device supported!\n");
                    break;
            }
        }

        free(pQueueFamilyProperties);
        if(supportsGraphics == 0) {
            fprintf(stdout, "Queue families not supported on this gpu");
            continue;
        }

        // Required extension check

        uint32_t requiredDeviceExtensionCount = 1;
        const char* requiredDeviceExtension[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        

        uint32_t deviceExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &deviceExtensionCount, NULL);

        VkExtensionProperties *deviceExtensions = malloc(deviceExtensionCount * sizeof(VkExtensionProperties));
        vkEnumerateDeviceExtensionProperties(physicalDevices[i], NULL, &deviceExtensionCount, deviceExtensions);

        int allExtensionsSupported = 1;

        for (uint32_t j = 0; j < requiredDeviceExtensionCount; j++) {
            int extSupported = 0;
            for (uint32_t d = 0; d < deviceExtensionCount; d++) {
                if (strcmp(deviceExtensions[d].extensionName, requiredDeviceExtension[j]) == 0) {
                    extSupported = 1;
                    break;
                }
            }
            if(extSupported == 0) {
                allExtensionsSupported = 0;
                break;
            }
        }

        free(deviceExtensions);

        if (!allExtensionsSupported) {
            continue;
        }

        // Required features
        VkPhysicalDeviceVulkan11Features vulkan11Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
            .pNext = NULL,
        };
        
        VkPhysicalDeviceVulkan13Features vulkan13Features = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
            .pNext = &vulkan11Features,
        };

        VkPhysicalDeviceExtendedDynamicStateFeaturesEXT edsFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
            .pNext = &vulkan13Features
        };

        VkPhysicalDeviceFeatures2 features2 = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &edsFeatures,
        };

        vkGetPhysicalDeviceFeatures2(physicalDevices[i], &features2);

        int supportsRequiredFeatures = vulkan11Features.shaderDrawParameters && vulkan13Features.dynamicRendering && edsFeatures.extendedDynamicState;

        if (!supportsRequiredFeatures)
            continue;

        vkGetPhysicalDeviceFeatures2(physicalDevices[i], pFeatures);

        if(physicalDeviceProperties->properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
            score += 1000;
        score += physicalDeviceProperties->properties.limits.maxImageDimension2D;

        if (!pFeatures->features.geometryShader) {
            fprintf(stdout, "geometryShader not supported!\n");
            continue;
        }
        fprintf(stdout, "Made it here!\n");
        if(highest_score <= score) {
            selectedDeviceIndex = i;
            highest_score = score;
        }
    }

    if(selectedDeviceIndex == -1) {
        fprintf(stderr, "Failed to find adequate physical device\n");
        exit(1);
    }
    else {
        *physicalDevice = physicalDevices[selectedDeviceIndex];
    }
    
    free(physicalDeviceProperties);
    free(pFeatures);
}

void createLogicalDevice(HelloTriangleApplication *app)
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties2(app->physicalDevice, &queueFamilyCount, NULL);

    VkQueueFamilyProperties2 *queueFamilies = malloc(queueFamilyCount * sizeof(VkQueueFamilyProperties2));

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        queueFamilies[i].sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2;
        queueFamilies[i].pNext = NULL;
    }

    vkGetPhysicalDeviceQueueFamilyProperties2(app->physicalDevice, &queueFamilyCount, queueFamilies);

    int queueFamilyWithGraphicsBitSetIndex = -1;
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilyWithGraphicsBitSetIndex = i;
            break;
        }
    }

    if(queueFamilyWithGraphicsBitSetIndex == -1) {
        fprintf(stderr, "No queueFamily with Graphics\n");
        exit(1);
    }

    VkPhysicalDeviceVulkan11Features vulkan11Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = NULL,
        .shaderDrawParameters = VK_TRUE,
    };
    
    VkPhysicalDeviceVulkan13Features vulkan13Features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
        .pNext = &vulkan11Features,
        .dynamicRendering = VK_TRUE,
    };

    VkPhysicalDeviceExtendedDynamicStateFeaturesEXT edsFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
        .pNext = &vulkan13Features,
        .extendedDynamicState = VK_TRUE,
    };

    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &edsFeatures,
    };

    float queuePriority = 0.5f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyWithGraphicsBitSetIndex,
        .pNext = NULL,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    uint32_t requiredDeviceExtensionCount = 1;
    const char* requiredDeviceExtension[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VkDeviceCreateInfo deviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features2,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &deviceQueueCreateInfo,
        .enabledExtensionCount = requiredDeviceExtensionCount,
        .ppEnabledExtensionNames = requiredDeviceExtension,
    };

    vkCreateDevice(app->physicalDevice, &deviceCreateInfo, NULL, &app->device);
    vkGetDeviceQueue(app->device, queueFamilyWithGraphicsBitSetIndex, 0, &app->graphicsQueue);

    free(queueFamilies);
}

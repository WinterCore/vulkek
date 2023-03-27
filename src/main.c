#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/vec4.h>
#include <cglm/mat4.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

const char *validationLayers[] = {
    "VK_LAYER_KHRONOS_validation",
};
uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else 
const bool enableValidationLayers = true;
#endif

typedef struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyExists;
    bool presentFamilyExists;
} QueueFamilyIndices;

typedef struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    VkSurfaceFormatKHR **formats;
    VkPresentModeKHR **presentModes;
} SwapChainSupportDetails;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    

    return details;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    QueueFamilyIndices indices = { .graphicsFamilyExists = false, .presentFamilyExists = false };

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, NULL);
    VkQueueFamilyProperties queueFamilies[queueFamilyCount];
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies);


    for (uint32_t i = 0; i < queueFamilyCount; i += 1) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            indices.graphicsFamilyExists = true;
        }

        if (presentSupport) {
            indices.presentFamily = i;
            indices.presentFamilyExists = true;
        }

        i += 1;
    }

    return indices;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    printf("%d available validation layers\n", layerCount);
    printf("%d required validation layers\n", validationLayerCount);

    for (uint32_t i = 0; i < layerCount; i += 1) {
        printf("%s\n", availableLayers[i].layerName);
    }

    for (uint32_t i = 0; i < validationLayerCount; i += 1) {
        bool layerFound = false;

        for (uint32_t j = 0; j < layerCount; j += 1) {
            if (strcmp(validationLayers[i], availableLayers[j].layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (! layerFound) {
            return false;
        }
    }
   
    return true;
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *window = glfwCreateWindow(800, 600, "Vulkan window", NULL, NULL);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);
    VkExtensionProperties extensionProperties[extensionCount];
    vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensionProperties);

    printf("%d extensions supported\n", extensionCount);

    for (uint32_t i = 0; i < extensionCount; i += 1) {
        printf("%s\n", extensionProperties[i].extensionName);
    }

    if (enableValidationLayers && ! checkValidationLayerSupport()) {
        fprintf(stderr, "[ERROR]: Not all requested validation layers are supported!");
        exit(EXIT_FAILURE);
    }
    
    fflush(stdout);

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "Vulkek Demo",
        .applicationVersion = VK_API_VERSION_1_3,
        .pEngineName = "No Engine",
        .engineVersion = VK_API_VERSION_1_3,
        .apiVersion = VK_API_VERSION_1_3,
    };

    uint32_t glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = glfwExtensionCount,
        .ppEnabledExtensionNames = glfwExtensions,
    };

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = validationLayerCount;
        createInfo.ppEnabledLayerNames = validationLayers;
    } else {
        createInfo.enabledLayerCount = 0;
    }

    VkInstance instance;
    VkResult createInstanceResult = vkCreateInstance(&createInfo, NULL, &instance);
    if (createInstanceResult != VK_SUCCESS) {
        fprintf(stderr, "[ERROR]: Failed to create vulkan instance, %d", createInstanceResult);
        exit(EXIT_FAILURE);
    }

    

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    
    if (deviceCount == 0) {
        fprintf(stderr, "[ERROR]: Failed to find GPUs with Vulkan support");
        exit(EXIT_FAILURE);
    }

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance, window, NULL, &surface) != VK_SUCCESS) {
        fprintf(stderr, "[ERROR]: Failed to create window surface!");
        exit(EXIT_FAILURE);
    }

    VkPhysicalDevice physicalDevices[deviceCount];
    vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices);
    printf("Found %d physical devices\n", deviceCount);


    // TODO: This is hard coded for now
    VkPhysicalDevice physicalDevice = physicalDevices[0];

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    VkDevice device;
    
    if (! indices.graphicsFamilyExists || ! indices.presentFamilyExists) {
        fprintf(stderr, "[ERROR]: Count not find a capable graphics queue");
        exit(EXIT_FAILURE);
    }

    float queuePriority = 1.0f;


    VkDeviceQueueCreateInfo queueCreateInfos[2];

    queueCreateInfos[0] = (VkDeviceQueueCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.graphicsFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    queueCreateInfos[1] = (VkDeviceQueueCreateInfo) {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = indices.presentFamily,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority,
    };

    const char *requiredExtensions[] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    VkPhysicalDeviceFeatures deviceFeatures = {VK_FALSE};
    VkDeviceCreateInfo logicalDeviceCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pQueueCreateInfos = queueCreateInfos,
        .queueCreateInfoCount = 2,
        .pEnabledFeatures = &deviceFeatures,
        .enabledExtensionCount = 1,
        .ppEnabledExtensionNames = requiredExtensions,
    };

    if (enableValidationLayers) {
        logicalDeviceCreateInfo.enabledLayerCount = validationLayerCount;
        logicalDeviceCreateInfo.ppEnabledLayerNames = validationLayers;
    } else {
        logicalDeviceCreateInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &logicalDeviceCreateInfo, NULL, &device) != VK_SUCCESS) {
        fprintf(stderr, "[ERROR]: Failed to create logical device!");
        exit(EXIT_FAILURE);
    }

    VkQueue graphicsQueue;
    vkGetDeviceQueue(device, indices.graphicsFamily, 0, &graphicsQueue);

    VkQueue presentQueue;
    vkGetDeviceQueue(device, indices.presentFamily, 0, &presentQueue);

    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

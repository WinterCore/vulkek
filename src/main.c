#include "aids.h"
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

#include "aids.c"

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
    VkSurfaceFormatKHR *formats;
    VkPresentModeKHR *presentModes;
    uint32_t formatCount;
    uint32_t presentModeCount;
} SwapChainSupportDetails;

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
    SwapChainSupportDetails details;

    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, NULL);
    details.formats = malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats);
    details.formatCount = formatCount;

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, NULL);
    details.presentModes = malloc(presentModeCount * sizeof(VkPresentModeKHR));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes);
    details.presentModeCount = presentModeCount;

    return details;
}

const VkSurfaceFormatKHR *chooseSwapSurfaceFormat(const VkSurfaceFormatKHR *formats, const uint32_t formatsCount) {
    for (uint32_t i = 0; i < formatsCount; i += 1) {
        const VkSurfaceFormatKHR *format = &formats[i];

        if (format->format == VK_FORMAT_B8G8R8A8_SRGB &&
            format->colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {

            return format;
        }
    }

    return &formats[0];
}

uint32_t clamp(uint32_t d, uint32_t min, uint32_t max) {
    const uint32_t t = d < min ? min : d;
    return t > max ? max : t;
}

VkExtent2D chooseSwapExtent(GLFWwindow *window, const VkSurfaceCapabilitiesKHR *capabilities) {
    if (capabilities->currentExtent.width != UINT32_MAX) {
        return capabilities->currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    VkExtent2D actualExtent = {
        (uint32_t) width,
        (uint32_t) height,
    };

    actualExtent.width = clamp(
        actualExtent.width,
        capabilities->minImageExtent.width,
        capabilities->maxImageExtent.width
    );

    actualExtent.height = clamp(
        actualExtent.height,
        capabilities->minImageExtent.height,
        capabilities->maxImageExtent.height
    );

    return actualExtent;
}

VkPresentModeKHR chooseSwapPresentMode(const VkPresentModeKHR *presentModes, const uint32_t presentModeCount) {
    for (uint32_t i = 0; i < presentModeCount; i += 1) {
        VkPresentModeKHR presentMode = presentModes[i];

        if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return presentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
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

VkShaderModule createShaderModule(VkDevice device, FileData *fileData) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = fileData->length,
        .pCode = (const uint32_t *) fileData->data,
    };

    VkShaderModule shaderModule;

    if (vkCreateShaderModule(device, &createInfo, NULL, &shaderModule) != VK_SUCCESS) {
        fprintf(stderr, "[ERROR]: Failed to create shader module!");
        exit(EXIT_FAILURE);
    }

    return shaderModule;
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


    SwapChainSupportDetails swapChainDetails = querySwapChainSupport(physicalDevice, surface);
    
    if (
        ! indices.graphicsFamilyExists ||
        ! indices.presentFamilyExists ||
        swapChainDetails.presentModeCount == 0 ||
        swapChainDetails.formatCount == 0) {

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
    
    const VkSurfaceFormatKHR *surfaceFormat = chooseSwapSurfaceFormat(swapChainDetails.formats, swapChainDetails.formatCount);;
    const VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainDetails.presentModes, swapChainDetails.presentModeCount);
    uint32_t imageCount = clamp(
        swapChainDetails.capabilities.minImageCount + 1,
        1,
        swapChainDetails.capabilities.maxImageCount
    );

    VkExtent2D swapchainExtent = chooseSwapExtent(window, &swapChainDetails.capabilities);

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat->format,
        .imageColorSpace = surfaceFormat->colorSpace,
        .imageExtent = swapchainExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = swapChainDetails.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    uint32_t queueFamilyIndices[2] = {
        indices.graphicsFamily,
        indices.presentFamily,
    };

    if (indices.graphicsFamily != indices.presentFamily) {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = NULL;
    }

    VkSwapchainKHR swapchain;

    if (vkCreateSwapchainKHR(device, &swapChainCreateInfo, NULL, &swapchain) != VK_SUCCESS) {
        fprintf(stderr, "[ERROR]: Failed to create swap chain");
        exit(EXIT_FAILURE);
    }
    printf("Created swap chain successfully!");

    VkImage *swapChainImages;
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, NULL);
    swapChainImages = malloc(imageCount * sizeof(VkImage));
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapChainImages);
    VkFormat swapChainImageFormat = surfaceFormat->format;

    VkImageView *swapchainImageViews = malloc(imageCount * sizeof(VkImageView));

    for (size_t i = 0; i < imageCount; i += 1) {
        VkImageViewCreateInfo imageViewCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = swapChainImages[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = swapChainImageFormat,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        };

        if (vkCreateImageView(device, &imageViewCreateInfo, NULL, &swapchainImageViews[i]) != VK_SUCCESS) {
            fprintf(stderr, "[ERROR]: Failed to create image views!");
            exit(EXIT_FAILURE);
        }
    }

    
    FileData vertShaderCode = read_file("./src/shaders/vert.spv");
    FileData fragShaderCode = read_file("./src/shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(device, &vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(device, &fragShaderCode);

    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

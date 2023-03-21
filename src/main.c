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
    "VK_LAYER_KHRONOS_validation"
};
uint32_t validationLayerCount = sizeof(validationLayers) / sizeof(validationLayers[0]);

#ifndef NDEBUG
const bool enableValidationLayers = false;
#else 
const bool enableValidationLayers = true;
#endif

bool checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, NULL);
    VkLayerProperties availableLayers[layerCount];
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);
    printf("%d available validation layers\n", layerCount);

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


    while (! glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    vkDestroyInstance(instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

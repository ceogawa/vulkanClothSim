#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "Debugging.hpp"
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp

// This Vulkan Project was built using https://vulkan-tutorial.com/Introduction as a foundation
// Claire Ogawa and Aidan Ream

// VULKAN INFO AND TIPS
// Vulkan designed for minimal driver overhead
// 
// Vulkan tends to use structs to store/update info rather than function parameters
//      object creation function parameters in Vulkan follow is:
//      1. Pointer to struct with creation info
//      2. Pointer to custom allocator callbacks (we are using default memory allocators)
//      3. Pointer to the variable that stores the handle to the new object

// https://docs.vulkan.org/spec/latest/chapters/extensions.html#extendingvulkan-extensions
// Extensions may define new Vulkan commands, structures, and enumerants. 
// Validation layers are optional components that hook into Vulkan function calls to apply additional operations.
//  - help us with error handling


const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

// configuration variables to specify which layers to enable/disable
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Application {
public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    GLFWwindow* window = nullptr;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // init physical device/graphics card
    VkDevice device; //Logical Device
    VkSurfaceKHR surface; // window surface to screen
    VkQueue graphicsQueue; //Graphics queue and Present queue families are often the same, but hardware dependent so we handle both.
    VkQueue presentQueue;
    VkSwapchainKHR swapChain; //Swap chain is how vulkan handles the frames in order- framebuffer settings and vsync settings etc
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;

    const std::vector<const char*> deviceExtensions = {
           VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // enable Vulkan SDK validation layers
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation" // bundled layer
    };

    void initWindow() {
        glfwInit(); // inits the library

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable defaulting to OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disabke window resizing for now

        // window(width, height, title, specify monitor, openglOnly)
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // init default window
    }

    // creates instance of vulkan (connection between app and the Vulkan library)
    void createInstance() {

        if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        // specify struct info
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // unsigned int - version number of the app (major, minor, patch)
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Tells the Vulkan driver which global extensions and validation layers we want to use.
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // now we are able to enable multiple validation layers if in debug mode
        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        checkSupportedExtensions(); 

        // populate instance attribute
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create instance.");
        }


    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    // checks what extensions are supported by vulkan
    void checkSupportedExtensions() {
        uint32_t extensionCount = 0;
        std::vector<VkExtensionProperties> extensions(extensionCount); // an array of VkExtensionProperties to store extension details
        // takes in (filter extensions by layer, &numOfExtensions, arr of extension details)
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        // document the number of available extensions
        std::cout << "available vulkan extensions: " << extensionCount << "\n";
    }
    
// Vulkan is a platform agnostic API, so we need extension to interface with the window system
// glfw can tell us which extensions we need
    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
       
        std::cout << "number of required glfw extensions: " << glfwExtensionCount << "\n";

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        //Check if device has queue family funcionality we require
        QueueFamilyIndices indices = findQueueFamilies(device);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        return indices.isComplete() && extensionsSupported && swapChainAdequate;

    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        //Checks if the hardware has a color mode and a presentation mode supported
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
        
    }

    // check for suitable graphics card, selects one
    void pickPhysicalDevice() { 
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr); // query number of graphics card

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!"); // this would be bad and sad
        }

        std::vector<VkPhysicalDevice> devices(deviceCount); // holds all device handles
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device : devices) { // check all devices for suitable graphics card
            if (isDeviceSuitable(device)) {
                physicalDevice = device;
                break;
            }
        }
        
        if (physicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }

    }

    //Creating a logical device to interface with the physical device
    // Currently configures for multiple queues, even if they are likely the same queues
    void createLogicalDevice() {
        //Create device queue info struct to initialize the vulkan object
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
        
        //assign priorities to queues to influence the scheduling of command buffer execution using floats between 0.0 and 1.0
        //This is required even if there is only a single queue:
        float queuePriority = 1.0f; // TODO look into meaning -c
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{}; 
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        //This is where we specify the physical device features we're gonna be using 
        //TODO: I think we will likely need to come back to this to activate compute shaders -A
        VkPhysicalDeviceFeatures deviceFeatures{};

        // creating the logical device struct
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
         
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data(); //pointer to the queue info

        createInfo.pEnabledFeatures = &deviceFeatures; //pointer to the device features

        //Set up validation layers to work with older versions of Vulkan
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }

        // create logical device
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        // retrieves queue handle
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

    }

    //Queue Families set up
    //A queue family is essentially groups on the GPU hardware that are designed to be handled together
    // Examples include geometry, computes, etc

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily; // init for drawing to buffer 
        std::optional<uint32_t> presentFamily; // init queue for writing to surface 
       
        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }

    };

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
       //Investigates the Hardware for queue families
        QueueFamilyIndices indices;
        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i; // init for drawing to buff
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport) {
                indices.presentFamily = i; // init for drawing to surface TODO check for combined support -c
            }

            if (indices.isComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    //Checking for swap chain support
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        //We're checking what the hardware allows us to do with swapchains
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        //Physcial Device surface formats supported
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        //Present Mode Checking
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    //Surface Format
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        //Loop through availible formats and select BGRa format
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        //You COULD search through availible formats for the best one but anbgtft
        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        //Present modes have positives and negatives, if we are running into visual bugs we should test with others
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                //Mailbox mode 
                //Instead of blocking the application when the queue is full,
                //the images that are already queued are simply replaced with the newer ones
                return availablePresentMode;
            }
        }
        //Traditional VSync, availible on all hardware
        return VK_PRESENT_MODE_FIFO_KHR;
    }


    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        //How is our screen working with pixels
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        //Defined above
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats); 
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        //Sets the number of images in the swapchain, 0 is a special value meaning no max, we are doing a specified number 
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        //Making the struct to create the swapchain vulkan object
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;

        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        //Updating the Graphics Queue Family with our new information
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }

        //Lets us do transformations on the images that are inside a given buffer, i.e. Rotations or flipping
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        //Vulkan has options to blend multiple windows color values together, we are just using opaque windows (standard)
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        //If we resize a window, we might need to make a new swap chain, this references the old one
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createImageViews() {
        swapChainImageViews.resize(swapChainImages.size()); // to fit all img views
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{}; // per view
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i]; // cur img
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; // specifies how to treat imgs (1d tex/2d tex, etc.)
            createInfo.format = swapChainImageFormat; 

            // components can be used for channel mapping, this is the default mapping 
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            // img purpose and which part of img
            // imgs used as COLOR TARGETS
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1; // default, not stereographic...

            // create single image view, store in array
            if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image views!");
            }

        }

    }



    // connects application to vulkan
    void initVulkan() {
        createInstance();
        createSurface(); // platform agnostic with GLFW
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain(); //  get format, present mode, extent
        createImageViews(); // sets up using images as textures
    }

    // renders a single frame 
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        // CLEAN UP ALL OBJECTS BEFORE DESTROYING INSTANCE
        for (auto imageView : swapChainImageViews) {
            vkDestroyImageView(device, imageView, nullptr); // manual cleanup because manual creation
        }

        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr); 
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr); // nullptr is optional allocator callback
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

int main() {
    // create instance of sample app
    Application app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n"; // throw exception if app exe fails
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    //sampleMain(); // TEST MAIN/POPUP WINDOW
}
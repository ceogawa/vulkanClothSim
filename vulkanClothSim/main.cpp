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
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // init physical device/graphics card
    VkDevice device; //Logical Device
    VkSurfaceKHR surface; // window surface to screen
    VkQueue graphicsQueue;
    VkQueue presentQueue;

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
        //Check if the machine has a discrete GPU to use
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        //Check if device has queue family funcionality we require
        QueueFamilyIndices indices = findQueueFamilies(device);
       
        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && indices.isComplete();

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
        //Create device queue info struct
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        ///*assign priorities to queues to influence the scheduling of command buffer execution using floats between 0.0 and 1.0
        //This is required even if there is only a single queue:*/
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
        createInfo.enabledExtensionCount = 0;

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

    /*Queue Families set up
    *A queue family is essentially groups on the GPU hardware that are designed to be handled together
    * Examples include geometry, computes, etc
    */

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

    // connects application to vulkan
    void initVulkan() {
        createInstance();
        createSurface(); // platform agnostic with GLFW
        pickPhysicalDevice();
        createLogicalDevice();

    }

    // renders a single frame 
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        // CLEAN UP ALL OBJECTS BEFORE DESTROYING INSTANCE
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
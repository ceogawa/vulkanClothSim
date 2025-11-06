#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

// VULKAN INFO AND TIPS
// Vulkan tends to use structs to store/update info rather than function parameters
//      object creation function parameters in Vulkan follow is:
//      1. Pointer to struct with creation info
//      2. Pointer to custom allocator callbacks (we are using default memory allocators)
//      3. Pointer to the variable that stores the handle to the new object

// TODO look more into "extensions" and what this means...

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

using namespace std;

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

    void initWindow() {
        glfwInit(); // inits the library

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable defaulting to OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disabke window resizing for now

        // window(width, height, title, specify monitor, openglOnly)
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // init default window
    }

    // creates instance of vulkan (connection between app and the Vulkan library)
    void createInstance() {
        VkApplicationInfo appInfo{};
        // specify struct info
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // unsigned int - version number of the app (major, minor, patch)
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        // Tells the Vulkan driver which global extensions and validation layers we want to use.
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        // Vulkan is a platform agnostic API, so we need extension to interface with the window system
        // glfw can tell us which extensions we need
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // retrieve extensions

        createInfo.enabledExtensionCount = glfwExtensionCount; // update struct
        createInfo.ppEnabledExtensionNames = glfwExtensions;

        cout << "number of required glfw extensions: " << glfwExtensionCount << endl;
        // TODO for now we have 0 enabled validation layers
        createInfo.enabledLayerCount = 0;

        checkSupportedExtensions(); 

        // populate instance attribute
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw runtime_error("Failed to create instance.");
        }
    }

    // checks what extensions are supported by vulkan
    void checkSupportedExtensions() {
        uint32_t extensionCount = 0;
        std::vector<VkExtensionProperties> extensions(extensionCount); // an array of VkExtensionProperties to store extension details
        // takes in (filter extensions by layer, &numOfExtensions, arr of extension details)
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        // document the number of available extensions
        std::cout << "available vulkan extensions: " << extensionCount << endl;
    }
    
    // connects application to vulkan
    void initVulkan() {
        createInstance();
    }

    // renders a single frame 
    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
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
        std::cerr << e.what() << std::endl; // throw exception if app exe fails
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    //sampleMain(); // TEST MAIN/POPUP WINDOW
}
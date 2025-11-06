#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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

    void initWindow() {
        glfwInit(); // inits the library

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // disable defaulting to OpenGL
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // disabke window resizing for now

        // window(width, height, title, specify monitor, openglOnly)
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr); // init default window
    }

    void initVulkan() {

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


int sampleMain() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", nullptr, nullptr);

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::cout << extensionCount << " extensions supported\n";

    glm::mat4 matrix;
    glm::vec4 vec;
    auto test = matrix * vec;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}

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
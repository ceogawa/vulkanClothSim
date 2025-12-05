#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
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
#include <fstream> // Necessary for file management
#include <array>

struct Vertex {
    glm::vec2 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        //A vertex binding describes at which rate to load data from memory throughout the vertices
        VkVertexInputBindingDescription bindingDescription{};
        // The binding parameter specifies the index of the binding in the array of bindings
        bindingDescription.binding = 0;
        // Stride parameter specifies the number of bytes from one entry to another
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        // The binding parameter specifies the index of the binding in the array of bindings
        attributeDescriptions[0].binding = 0;//Where its getting the data from
        //Vertex Shader inPosition layout(location=0)
        attributeDescriptions[0].location = 0; //Where its sending it in the vertex shader
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        //Color
        attributeDescriptions[1].binding = 0;//Where its getting the data from
        //Vertex Shader inColor layout(location=1)
        attributeDescriptions[1].location = 1; //Where its sending it in the vertex shader
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        return attributeDescriptions;
    }

};

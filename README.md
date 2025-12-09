# vulkanClothSim
Written in C++17

**Dependencies:** 
- GLFW
- GLM
- Vulkan SDK
- tiny_obj_loader
- stb_image.h

Download and install latest version of Vulkan SDK

**Compiling Shaders:** 
Create a compile.bat file in the resources folder that has the general format of: 

    C:\Users\pathname\vulkan\Bin\glslc.exe cloth.vert -o vert.spv\
    C:\Users\pathname\vulkan\Bin\glslc.exe cloth.frag -o frag.spv\
    C:\Users\pathname\vulkan\Bin\glslc.exe cloth.comp -o comp.spv\
    pause\

Run the bat file to compile the vertex, fragment, and compute shaders into SPIR-V format 

**Building and Running Project:** 
Using Microsoft Visual Studio open the project solution file and adjust the project dependency settings to point
to the folder where you keep the Vulkan SDK, GLM, GLFW and file headers. Run the sln file. 

**Paper Inspiration and Addition:** 
The paper we used as inspiration for our project implements a mass-spring cloth simulation in OpenGL using compute shaders. 
Using vulkan tutorials and other youtube video resources on compute shaders, we rebuilt a cloth simulation from scratch using Vulkan. 
Vulkan has much finer grain control over the GPU compared to OpenGL, with potential for performance optimization. We also added
obj loader support into our project. Our project successfully completes the goal of building our own mass-spring cloth simulation
using compute shaders in Vulkan.




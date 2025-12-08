# vulkanClothSim
Written in C++17

Dependencies:
- GLFW
- GLM
- Vulkan SDK
- tiny_obj_loader
- stb_image.h

Download and install latest version of Vulkan SDK

Building Project:
Using Microsoft Visual Studio open the project solution file and adjust the project dependency settings to point
to the folder where you keep the Vulkan SDK, GLM, GLFW and file headers

Compiling Shaders:
Create a compile.bat file in the resources folder that has the general format of:
    C:\Users\***\vulkan\Bin\glslc.exe cloth.vert -o vert.spv
    C:\Users\***\vulkan\Bin\glslc.exe cloth.frag -o frag.spv
    C:\Users\***\vulkan\Bin\glslc.exe cloth.comp -o comp.spv
    pause

Compile the vertex, fragment, and compute shaders into spir-v format

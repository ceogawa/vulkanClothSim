# vulkanClothSim
Written in C++20

Built by Claire Ogawa and Aidan Ream
1st Place Project in Graduate-level Graphics Course
DEMO: https://fahimhkhan.github.io/cgtops/572_f25/ceogawa_acream/

Refactoring effort By Aidan Ream

Note to Claire:
This made changes to the project building and linking- changing it to Cmake instead
of inate Visual Studio Linker settings. Also means vcpkg is needed, and Cmake is needed.
You may need to adjust things to fix build/project errors that this may or may not introduce.
Note: I have the base refactor in a seperate git repo, so feel free to break things as you like 

also, If you want to try and get this working I would back up your local files, the sln depends on
the vcxproj.user files and such

When you first open the project, CMake will run and it will pause on
[CMake] Fetching registry information from https://github.com/microsoft/vcpkg (HEAD)...
For a couple minutes. Just let it load and if there is an error we can solve from there.

Your files should only contain

resources/
src/
.gitignore
CMakeLists.txt
CMakePresets.json
README.md
vcpkg.JSON

you will need to have an enviroment variable pointing at your downleaded vcpkg
called VCPKG_ROOT I think

Cmake will automatically download and link the dependancies (which is part of why I made this change)

Note: For shader work you will still need compile.bat, it is still in the gitignore because it links
to the compiler in your own VulkanSDK. Would probably be worth seeing if there is someway to make this
more transportable too, but for now its fine.

Dependencies:
- GLFW
- GLM
- Vulkan SDK
- TinyOBJLoader
- stb_image.h

CmakeLists.txt and Vcpkg installer shoudl install and link all needed dependancies for you.

To run:

Run Compile.bat to convert cloth.vert, cloth.frag, and cloth.comp into sprv

Compiling Shaders: Create a compile.bat file in the resources folder that has the general format of:

C:\Users\pathname\vulkan\Bin\glslc.exe cloth.vert -o vert.spv
C:\Users\pathname\vulkan\Bin\glslc.exe cloth.frag -o frag.spv
C:\Users\pathname\vulkan\Bin\glslc.exe cloth.comp -o comp.spv
pause
Run the bat file to compile the vertex, fragment, and compute shaders into SPIR-V format


Build project using Cmake

run excecutable from /out/ directory in project
.\build\x64-release\VulkanBaseProject.exe


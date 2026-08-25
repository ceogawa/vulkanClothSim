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
:)

Dependencies:
- GLFW
- GLM
- Vulkan SDK
- TinyOBJLoader

To run:

Run Compile.bat to convert cloth.vert, cloth.frag, and cloth.comp into sprv

Build project using Cmake

run excecutable from /out/ directory in project
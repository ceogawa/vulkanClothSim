#version 450

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    // vec3 Gravity = vec3(0, -10, 0); //Vulkan y is upsidown, maybe change?
    // float ParticleMass = 0.1; 
    // float ParticleInvMass = 1.0 / 0.1; //Inverse Mass
    // float SpringK = 2000.0;
    // float RestLengthHoriz;
    // float RestLengthVert;
    // float RestLengthDiag;
    // float DeltaT = 0.000005;
    // float DampingConst = 0.1;
} ubo;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main() {
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
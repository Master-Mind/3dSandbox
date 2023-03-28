#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

#include "Graphics/Graphics.h"

int main() {
    Graphics::Init();

    while (!Graphics::ShouldClose())
    {
        Graphics::Update();
    }

    Graphics::DeInit();

    return 0;
}
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <filesystem>
#include "Graphics/Graphics.h"

int main() {
    Graphics::Init();
    char *temp;
    size_t tempsize;
	errno_t err = _dupenv_s(&temp, &tempsize, "VK_INSTANCE_LAYERS");
    while (!Graphics::ShouldClose())
    {
        Graphics::Update();
    }

    Graphics::DeInit();

    return 0;
}
#include <stdio.h>

#include "GLFW/glfw3.h"

#define WIDTH 800
#define HEIGHT 600

void run();
GLFWwindow* initWindow();
void initVulkan();
void mainLoop(GLFWwindow* window);
void cleanup(GLFWwindow* window);

int main() {
    run();
    return 0;
}

void run()
{
    GLFWwindow *window = NULL;
    window = initWindow();
    if(!window)
    {
        // Failed to load window logic
    }
    initVulkan();
    mainLoop(window);
    cleanup(window);
}

GLFWwindow* initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    return glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", NULL, NULL);
}

void initVulkan()
{
}

void mainLoop(GLFWwindow* window)
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void cleanup(GLFWwindow* window)
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

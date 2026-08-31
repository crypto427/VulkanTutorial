#include <stdio.h>

void run();
void initWindow();
void initVulkan();
void mainLoop();
void cleanup();

int main() {
    printf("Hello world!\n");
    return 0;
}

void run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void initWindow()
{
}

void initVulkan()
{
}

void mainLoop()
{
}

void cleanup()
{
}

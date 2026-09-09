#include <stdio.h>
#include "GLFW/glfw3.h"

int main() {
    if (!glfwInit()) {
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Hello World!", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -2;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window)) {
    	glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

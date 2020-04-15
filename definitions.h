#pragma once
#define M_PI acos(-1.0)

// class definitions
enum class DrawMode : unsigned int {
    none,
    line,
    polygon
};

enum class Transformation : unsigned int {
    none,
    translation,
    scaling,
    rotation,
    reflectionX,
    reflectionY,
    reflectionOrigin,
    shearX,
    shearY
};

// function declarations
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processKeyboardInput(GLFWwindow* window);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void clearCoordinates();
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void insertCoordinates(float xpos, float ypos, bool temporary = false);
void characterCallback(GLFWwindow* window, unsigned int codepoint);
void clearCharacterBuffer();
void processTransformation(float x = 0.0f, float y = 0.0f);
void refreshBuffer();
void normalizeCoordinates(float *x, float *y);
#include <iostream>
#include <vector>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include "definitions.h"

// global variables
unsigned int SCR_WIDTH = 640;
unsigned int SCR_HEIGHT = 360;

DrawMode drawMode = DrawMode::line;
Transformation transformation = Transformation::none;

std::vector<float> linesCoordinates;
std::vector<float> polygonCoordinates;
std::vector<int> polygonIndexes = { 0 };
std::vector<float> transformationWindowCoordinates;

unsigned int VBO[3], VAO[3];
unsigned int vertexShader, fragmentShader, shaderProgram;

int main() {
    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // creating a window
    //GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics Simulator", glfwGetPrimaryMonitor(), NULL);
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics Simulator", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // initializing GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // cursor
    GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    glfwSetCursor(window, cursor);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // initialize vertex buffer object
    for (int i = 0; i < sizeof(VBO) / sizeof(unsigned int); ++i) {
        glGenBuffers(1, &VBO[i]);
    }

    // initialize vertex array object
    for (int i = 0; i < sizeof(VAO) / sizeof(unsigned int); ++i) {
        glGenVertexArrays(1, &VAO[i]);
    }

    // initialize vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // initialize fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // initialize shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // bind VAO and VBO
    for (int i = 0; i < sizeof(VAO) / sizeof(unsigned int); ++i) {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    }

    while (!glfwWindowShouldClose(window)) {
        // process keyboard input
        processKeyboardInput(window);

        //render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw
        glUseProgram(shaderProgram);

        if (transformation != Transformation::none) {
            glBindVertexArray(VAO[2]);
            glDrawArrays(GL_LINE_LOOP, 0, 4);
        }

        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_LINES, 0, (linesCoordinates.size() / 3) + 1);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_LINES, 0, (polygonCoordinates.size() / 3) + 2);


        // swap buffers and poll IO events
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    // terminate, unallocating resources
    glfwTerminate();
    return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void processKeyboardInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_DELETE) == GLFW_PRESS) {
        transformation = Transformation::none;
        drawMode = DrawMode::none;
        clearCoordinates();
    }
    else if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        drawMode = DrawMode::line;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        drawMode = DrawMode::polygon;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        transformation = Transformation::translation;
    }
    else if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
        transformation = Transformation::none;
        transformationWindowCoordinates.clear();
        glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        insertCoordinates((float)xpos, (float)ypos);
    }
}

void clearCoordinates() {
    linesCoordinates.clear();
    polygonCoordinates.clear();
    polygonIndexes.clear();
    polygonIndexes.push_back(0);
    transformationWindowCoordinates.clear();
    for (int i = 0; i < sizeof(VBO) / sizeof(unsigned int); ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    }
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (transformation != Transformation::none) {
        if (transformationWindowCoordinates.size() % 6 == 3) {
            // add coordinates temporarily
            insertCoordinates((float)xpos, (float)ypos, true);
        }
    }
    else if (drawMode == DrawMode::line) {
        if (linesCoordinates.size() % 6 == 3) {
            // add coordinates temporarily
            insertCoordinates((float)xpos, (float)ypos, true);
        }
    }
    else if (drawMode == DrawMode::polygon) {
        if (polygonCoordinates.size() > polygonIndexes.back()) {
            // add coordinates temporarily
            insertCoordinates((float)xpos, (float)ypos, true);
        }
    }
}

void insertCoordinates(float xpos, float ypos, bool temporary) {
    float xValue = (2.0f / (float)SCR_WIDTH) * xpos - 1;
    // taking the negative of the coordinate to flip Y-axis
    float yValue = -((2.0f / (float)SCR_HEIGHT) * ypos - 1);

    if (transformation != Transformation::none) {
        if (transformationWindowCoordinates.size() % 6 == 0) {
            transformationWindowCoordinates.clear();
            transformationWindowCoordinates.push_back(xValue);
            transformationWindowCoordinates.push_back(yValue);
            transformationWindowCoordinates.push_back(0.0f);
        }
        else {
            transformationWindowCoordinates.push_back(xValue);
            transformationWindowCoordinates.push_back(transformationWindowCoordinates[transformationWindowCoordinates.size() - 3]);
            transformationWindowCoordinates.push_back(0.0f);

            transformationWindowCoordinates.push_back(xValue);
            transformationWindowCoordinates.push_back(yValue);
            transformationWindowCoordinates.push_back(0.0f);

            transformationWindowCoordinates.push_back(transformationWindowCoordinates[transformationWindowCoordinates.size() - 9]);
            transformationWindowCoordinates.push_back(yValue);
            transformationWindowCoordinates.push_back(0.0f);
        }
        // draw
        if (transformationWindowCoordinates.size() % 6 == 0) {
            float* vertices = &transformationWindowCoordinates[0];
            glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * transformationWindowCoordinates.size(), vertices, GL_STATIC_DRAW);
        }

        if (temporary) {
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
            transformationWindowCoordinates.pop_back();
        }
    }
    else if (drawMode == DrawMode::line) {
        linesCoordinates.push_back(xValue);
        linesCoordinates.push_back(yValue);
        linesCoordinates.push_back(0.0f);

        // draw
        if (linesCoordinates.size() % 6 == 0) {
            float* vertices = &linesCoordinates[0];
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * linesCoordinates.size(), vertices, GL_STATIC_DRAW);
        }

        if (temporary) {
            linesCoordinates.pop_back();
            linesCoordinates.pop_back();
            linesCoordinates.pop_back();
        }
    }
    else if (drawMode == DrawMode::polygon) {
        if (temporary) {
            if (polygonCoordinates.size() > polygonIndexes.back() + 3 && abs(xValue - polygonCoordinates[polygonIndexes.back()]) < 0.05f && abs(yValue - polygonCoordinates[polygonIndexes.back() + 1]) < 0.05f) {
                polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                polygonCoordinates.push_back(0.0f);

                polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back()]);
                polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back() + 1]);
                polygonCoordinates.push_back(0.0f);
            }
            else {
                if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    polygonCoordinates.push_back(0.0f);
                }
                polygonCoordinates.push_back(xValue);
                polygonCoordinates.push_back(yValue);
                polygonCoordinates.push_back(0.0f);
            }
        }
        else {
            if ((polygonCoordinates.size() - polygonIndexes.back()) % 6 == 3) {
                polygonCoordinates.push_back(xValue);
                polygonCoordinates.push_back(yValue);
                polygonCoordinates.push_back(0.0f);
            }
            else {
                if (polygonCoordinates.size() > polygonIndexes.back() + 3 && abs(xValue - polygonCoordinates[polygonIndexes.back()]) < 0.05f && abs(yValue - polygonCoordinates[polygonIndexes.back() + 1]) < 0.05f) {
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    polygonCoordinates.push_back(0.0f);
                    polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back()]);
                    polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back() + 1]);
                    polygonCoordinates.push_back(0.0f);
                    polygonIndexes.push_back(polygonCoordinates.size());
                }
                else {
                    if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
                        polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                        polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                        polygonCoordinates.push_back(0.0f);
                    }
                    polygonCoordinates.push_back(xValue);
                    polygonCoordinates.push_back(yValue);
                    polygonCoordinates.push_back(0.0f);
                }
            }
        }

        // draw lines
        if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
            float* vertices = &polygonCoordinates[0];
            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)* polygonCoordinates.size(), vertices, GL_STATIC_DRAW);
        }

        if (temporary) {
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
                polygonCoordinates.pop_back();
                polygonCoordinates.pop_back();
                polygonCoordinates.pop_back();
            }
        }
    }
}
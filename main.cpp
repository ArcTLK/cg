#include <iostream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaders.h"
#include "definitions.h"

// global variables
unsigned int SCR_WIDTH = 640;
unsigned int SCR_HEIGHT = 640;

DrawMode drawMode = DrawMode::line;
Transformation transformation = Transformation::none;

std::vector<float> linesCoordinates;
std::vector<float> polygonCoordinates;
std::vector<int> polygonIndexes = { 0 };
std::vector<std::vector<float> *> filledPolygonCoordinates;
std::vector<float> transformationWindowCoordinates;
std::vector<char> keyboardInput = { '\0' };

std::vector<unsigned int> VBO;
std::vector<unsigned int> VAO;
unsigned int vertexShader, fragmentShader, shaderProgram;

bool listenForKeyboardInput = false;

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

    // keyboard
    glfwSetCharCallback(window, characterCallback);

    // initialize vertex buffer object
    VBO.push_back(0);
    VBO.push_back(0);
    VBO.push_back(0);
    for (int i = 0; i < VBO.size(); ++i) {
        glGenBuffers(1, &VBO[i]);
    }

    // initialize vertex array object
    VAO.push_back(0);
    VAO.push_back(0);
    VAO.push_back(0);
    for (int i = 0; i < VAO.size(); ++i) {
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
    for (int i = 0; i < VAO.size(); ++i) {
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

        glBindVertexArray(VAO[0]);
        glDrawArrays(GL_LINES, 0, (linesCoordinates.size() / 3) + 1);
        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_LINES, 0, (polygonCoordinates.size() / 3) + 2);
        glBindVertexArray(VAO[2]);
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        for (int i = 3; i < VAO.size(); ++i) {
            glBindVertexArray(VAO[i]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, filledPolygonCoordinates[i - 3]->size() / 3);
        }

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
        refreshBuffer();
        drawMode = DrawMode::line;
    }
    else if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        refreshBuffer();
        drawMode = DrawMode::polygon;
    }
    else if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::translation;
    }
    else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::scaling;
    }
    else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::rotation;
    }
    else if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::reflectionX;
    }
    else if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::reflectionY;
    }
    else if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::reflectionOrigin;
    }
    else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::shearX;
    }
    else if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        refreshBuffer();
        transformation = Transformation::shearY;
    }
    else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        refreshBuffer();
        drawMode = DrawMode::floodFill;
    }
    else if (glfwGetKey(window, GLFW_KEY_END) == GLFW_PRESS) {
        transformation = Transformation::none;
        transformationWindowCoordinates.clear();
        refreshBuffer();
    }
    else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (listenForKeyboardInput) {
            if (transformation == Transformation::translation ||
                transformation == Transformation::scaling ||
                transformation == Transformation::rotation ||
                transformation == Transformation::shearX ||
                transformation == Transformation::shearY) {
                char* firstString = keyboardInput.data();
                char* secondString = NULL;
                for (int i = 0; i < keyboardInput.size(); ++i) {
                    if (keyboardInput[i] == ' ') {
                        keyboardInput[i] = '\0';
                        secondString = &keyboardInput[i + 1];
                        break;
                    }
                }
                float x = (float)strtod(firstString, (char**)NULL);
                float y = 0.0f;
                if (secondString != NULL) {
                    y = (float)strtod(secondString, (char**)NULL);
                }
                processTransformation(x, y);
            }
            listenForKeyboardInput = false;
            clearCharacterBuffer();
        }
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
    filledPolygonCoordinates.clear();
    for (int i = 0; i < filledPolygonCoordinates.size(); ++i) {
        filledPolygonCoordinates[i]->clear();
        delete filledPolygonCoordinates[i];
    }
    transformationWindowCoordinates.clear();
    for (int i = 0; i < VBO.size(); ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
    }
}

void refreshBuffer() {
    // make sure coordinates are always even
    if (linesCoordinates.size() % 6 == 3) {
        linesCoordinates.pop_back();
        linesCoordinates.pop_back();
        linesCoordinates.pop_back();
    }
    if (polygonCoordinates.size() != polygonIndexes.back()) {
        while (polygonCoordinates.size() != polygonIndexes.back()) {
            polygonCoordinates.pop_back();
        }
    }

    // clean
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
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
    float xValue = xpos;
    float yValue = ypos;
    normalizeCoordinates(&xValue, &yValue);

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

            if (!temporary) {
                if (transformation == Transformation::translation ||
                    transformation == Transformation::scaling ||
                    transformation == Transformation::rotation ||
                    transformation == Transformation::shearX ||
                    transformation == Transformation::shearY) {
                    // listen to keyboard input
                    listenForKeyboardInput = true;
                }
                else {
                    processTransformation();
                }
            }
        }
        // draw
        if (transformationWindowCoordinates.size() % 6 == 0) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * transformationWindowCoordinates.size(), &transformationWindowCoordinates[0], GL_STATIC_DRAW);
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
            glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * linesCoordinates.size(), &linesCoordinates[0], GL_STATIC_DRAW);
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
            glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float)* polygonCoordinates.size(), &polygonCoordinates[0], GL_STATIC_DRAW);
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
    else if (drawMode == DrawMode::floodFill) {
        int polygons = polygonIndexes.size() - 1;
        for (int i = 0; i < polygons; ++i) {
            bool inside = false;
            for (int k = polygonIndexes[i] / 3, j = (polygonIndexes[i + 1]) / 3 - 1; k < (polygonIndexes[i + 1]) / 3; j = k++) {
                if (((polygonCoordinates[k * 3 + 1] > yValue) != (polygonCoordinates[j * 3 + 1] > yValue)) &&
                    (xValue < (polygonCoordinates[j * 3] - polygonCoordinates[k * 3]) * (yValue - polygonCoordinates[k * 3 + 1]) / (polygonCoordinates[j * 3 + 1] - polygonCoordinates[k * 3 + 1]) + polygonCoordinates[k * 3])) {
                    inside = !inside;
                }
            }
            if (inside) {
                std::vector<float>* vector = new std::vector<float>;
                filledPolygonCoordinates.push_back(vector);
                for (int j = polygonIndexes[i]; j < polygonIndexes[i + 1]; ++j) {
                    filledPolygonCoordinates.back()->push_back(polygonCoordinates[j]);
                }

                VAO.push_back(0);
                VBO.push_back(0);
                glGenBuffers(1, &(VBO.back()));
                glGenVertexArrays(1, &(VAO.back()));
                glBindVertexArray(VAO.back());
                glBindBuffer(GL_ARRAY_BUFFER, VBO.back());
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * filledPolygonCoordinates.back()->size(), filledPolygonCoordinates.back()->data(), GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            }
        }
    }
}

void characterCallback(GLFWwindow* window, unsigned int codepoint) {
    if (listenForKeyboardInput) {
        keyboardInput.insert(--keyboardInput.end(), (char)codepoint);
    }
}

void clearCharacterBuffer() {
    keyboardInput.clear();
    keyboardInput.push_back('\0');
}

void processTransformation(float x, float y) {
    // get transformation window
    float xMin = std::min(transformationWindowCoordinates[0], transformationWindowCoordinates[3]);
    float xMax = std::max(transformationWindowCoordinates[0], transformationWindowCoordinates[3]);
    float yMin = std::min(transformationWindowCoordinates[1], transformationWindowCoordinates[7]);
    float yMax = std::max(transformationWindowCoordinates[1], transformationWindowCoordinates[7]);

    glm::mat4 trans;

    if (transformation == Transformation::translation) {        
        x /= SCR_WIDTH;
        y /= SCR_HEIGHT;
    }
    else if (transformation == Transformation::shearX) {
        x /= SCR_WIDTH;
    }
    else if (transformation == Transformation::shearY) {
        x /= SCR_HEIGHT;
    }
    else if (transformation == Transformation::rotation) {
        trans = glm::mat4(1.0f);
        trans = glm::rotate(trans, glm::radians(-x), glm::vec3(0.0, 0.0, 1.0));
    }

    int polygons = polygonIndexes.size() - 1;

    for (int i = 0; i < linesCoordinates.size(); i += 6) {
        if (linesCoordinates[i] >= xMin && linesCoordinates[i] <= xMax
            && linesCoordinates[i + 1] >= yMin && linesCoordinates[i + 1] <= yMax
            && linesCoordinates[i + 3] >= xMin && linesCoordinates[i + 3] <= xMax
            && linesCoordinates[i + 4] >= yMin && linesCoordinates[i + 4] <= yMax) {
            if (transformation == Transformation::reflectionX) {
                linesCoordinates[i + 1] = -linesCoordinates[i + 1];
                linesCoordinates[i + 4] = -linesCoordinates[i + 4];
            }
            else if (transformation == Transformation::reflectionY) {
                linesCoordinates[i] = -linesCoordinates[i];
                linesCoordinates[i + 3] = -linesCoordinates[i + 3];
            }
            else if (transformation == Transformation::reflectionOrigin) {
                linesCoordinates[i] = -linesCoordinates[i];
                linesCoordinates[i + 3] = -linesCoordinates[i + 3];
                linesCoordinates[i + 1] = -linesCoordinates[i + 1];
                linesCoordinates[i + 4] = -linesCoordinates[i + 4];
            }
            else if (transformation == Transformation::translation) {
                linesCoordinates[i] += x;
                linesCoordinates[i + 3] += x;
                linesCoordinates[i + 1] += y;
                linesCoordinates[i + 4] += y;
            }
            else if (transformation == Transformation::scaling) {
                linesCoordinates[i] *= x;
                linesCoordinates[i + 3] *= x;
                linesCoordinates[i + 1] *= y;
                linesCoordinates[i + 4] *= y;
            }
            else if (transformation == Transformation::rotation) {
                glm::vec4 vector = { linesCoordinates[i], linesCoordinates[i + 1], 0.0f, 1.0f };
                vector = vector * trans;
                linesCoordinates[i] = vector.x;
                linesCoordinates[i + 1] = vector.y;

                vector = { linesCoordinates[i + 3], linesCoordinates[i + 4], 0.0f, 1.0f };
                vector = vector * trans;
                linesCoordinates[i + 3] = vector.x;
                linesCoordinates[i + 4] = vector.y;
            }
            else if (transformation == Transformation::shearX) {
                linesCoordinates[i] += x * linesCoordinates[i + 1];
                linesCoordinates[i + 3] += x * linesCoordinates[i + 4];
            }
            else if (transformation == Transformation::shearY) {
                linesCoordinates[i + 1] += x * linesCoordinates[i];
                linesCoordinates[i + 4] += x * linesCoordinates[i + 3];
            }
        }
    }

    for (int i = 0; i < polygons; ++i) {
        bool inside = true;
        for (int j = polygonIndexes[i]; j < polygonIndexes[i + 1]; j += 3) {
            if (polygonCoordinates[j] < xMin || polygonCoordinates[j] > xMax || polygonCoordinates[j + 1] < yMin || polygonCoordinates[j + 1] > yMax) {
                inside = false;
                break;
            }
        }
        if (inside) {
            for (int j = polygonIndexes[i]; j < polygonIndexes[i + 1]; j += 3) {
                if (transformation == Transformation::reflectionX) {
                    polygonCoordinates[j + 1] = -polygonCoordinates[j + 1];
                }
                else if (transformation == Transformation::reflectionY) {
                    polygonCoordinates[j] = -polygonCoordinates[j];
                }
                else if (transformation == Transformation::reflectionOrigin) {
                    polygonCoordinates[j] = -polygonCoordinates[j];
                    polygonCoordinates[j + 1] = -polygonCoordinates[j + 1];
                }
                else if (transformation == Transformation::translation) {
                    polygonCoordinates[j] += x;
                    polygonCoordinates[j + 1] += y;
                }
                else if (transformation == Transformation::scaling) {
                    polygonCoordinates[j] *= x;
                    polygonCoordinates[j + 1] *= y;
                }
                else if (transformation == Transformation::rotation) {
                    glm::vec4 vector = { polygonCoordinates[j], polygonCoordinates[j + 1], 0.0f, 1.0f };
                    vector = vector * trans;
                    polygonCoordinates[j] = vector.x;
                    polygonCoordinates[j + 1] = vector.y;
                }
                else if (transformation == Transformation::shearX) {
                    polygonCoordinates[j] += x * polygonCoordinates[j + 1];
                }
                else if (transformation == Transformation::shearY) {
                    polygonCoordinates[j + 1] += x * polygonCoordinates[j];
                }
            }
        }
    }

    for (int i = 0; i < filledPolygonCoordinates.size(); ++i) {
        bool inside = true;
        for (auto j = filledPolygonCoordinates[i]->begin(); j != filledPolygonCoordinates[i]->end(); j += 3) {
            if (*j < xMin || *j > xMax || *(j + 1) < yMin || *(j + 1) > yMax) {
                inside = false;
                break;
            }
        }
        if (inside) {
            for (auto j = filledPolygonCoordinates[i]->begin(); j != filledPolygonCoordinates[i]->end(); j += 3) {
                if (transformation == Transformation::reflectionX) {
                    *(j + 1) = -*(j + 1);
                }
                else if (transformation == Transformation::reflectionY) {
                    *j = -*j;
                }
                else if (transformation == Transformation::reflectionOrigin) {
                    *j = -*j;
                    *(j + 1) = -*(j + 1);
                }
                else if (transformation == Transformation::translation) {
                    *j += x;
                    *(j + 1) += y;
                }
                else if (transformation == Transformation::scaling) {
                    *j *= x;
                    *(j + 1) *= y;
                }
                else if (transformation == Transformation::rotation) {
                    glm::vec4 vector = { *j, *(j + 1), 0.0f, 1.0f };
                    vector = vector * trans;
                    *j = vector.x;
                    *(j + 1) = vector.y;
                }
                else if (transformation == Transformation::shearX) {
                    *j += x * *(j + 1);
                }
                else if (transformation == Transformation::shearY) {
                    *(j + 1) += x * *j;
                }
            }
        }
    }

    if (!linesCoordinates.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * linesCoordinates.size(), &linesCoordinates[0], GL_STATIC_DRAW);
    }
    if (polygons > 0) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * polygonCoordinates.size(), &polygonCoordinates[0], GL_STATIC_DRAW);
    }
    for (int i = 0; i < filledPolygonCoordinates.size(); ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i + 3]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)* filledPolygonCoordinates[i]->size(), filledPolygonCoordinates[i]->data(), GL_STATIC_DRAW);
    }
}

void normalizeCoordinates(float *x, float *y) {
    *x = (2.0f / (float)SCR_WIDTH) * *x - 1;
    // flip Y coordinate
    *y = -((2.0f / (float)SCR_HEIGHT) * *y - 1);
}
#include <iostream>
#include <vector>
#include <math.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// vertex shader code
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";

// fragment shader code
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"    FragColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);\n"
"}\0";

// class definitions
enum class DrawMode : unsigned int {
    none,
    line,
    polygon
};

// function declarations
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void clearCoordinates();
void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
void insertCoordinates(float xpos, float ypos, bool temporary = false);

// global constants
const unsigned int SCR_WIDTH = 1366;
const unsigned int SCR_HEIGHT = 768;

// global variables
DrawMode drawMode = DrawMode::none;
std::vector<float> linesCoordinates;
std::vector<float> polygonCoordinates;
std::vector<int> polygonIndexes = { 0 };
unsigned int VBO[2], VAO[2];
unsigned int vertexShader, fragmentShader, shaderProgram;

int main() {
    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // creating a window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics Simulator", glfwGetPrimaryMonitor(), NULL);
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
    glGenBuffers(1, &VBO[0]);
    glGenBuffers(1, &VBO[1]);

    // initialize vertex array object
    glGenVertexArrays(1, &VAO[0]);
    glGenVertexArrays(1, &VAO[1]);

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
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

    while (!glfwWindowShouldClose(window)) {
        // process any input
        processInput(window);

        //render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw
        glUseProgram(shaderProgram);

        glBindVertexArray(VAO[1]);
        glDrawArrays(GL_LINES, 0, (polygonCoordinates.size() / 3) + 2);

        if (drawMode == DrawMode::line) {
            glBindVertexArray(VAO[0]);
            glDrawArrays(GL_LINES, 0, (linesCoordinates.size() / 3) + 2);
        }
        else if (drawMode == DrawMode::polygon) {
            glBindVertexArray(VAO[1]);
            glDrawArrays(GL_LINES, 0, (polygonCoordinates.size() / 3) + 2);
        }

        // swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // terminate, unallocating resources
    glfwTerminate();
    return 0;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    else if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
        drawMode = DrawMode::none;
        clearCoordinates();
    }
    else if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        drawMode = DrawMode::line;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        drawMode = DrawMode::polygon;
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
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    if (drawMode == DrawMode::line) {
        if (linesCoordinates.size() % 6 == 3) {
            // add coordinates temporarily
            insertCoordinates((float)xpos, (float)ypos);
            // draw line
            float* vertices = &linesCoordinates[0];
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * linesCoordinates.size(), vertices, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            // remove temporary coordinates
            linesCoordinates.pop_back();
            linesCoordinates.pop_back();
            linesCoordinates.pop_back();
        }
    }
    else if (drawMode == DrawMode::polygon) {
        if (polygonCoordinates.size() > polygonIndexes.back()) {
            // add coordinates temporarily
            insertCoordinates((float)xpos, (float)ypos, true);
            // draw line
            //float* vertices = &polygonCoordinates[0];
            //glBufferData(GL_ARRAY_BUFFER, sizeof(float) * polygonCoordinates.size(), vertices, GL_DYNAMIC_DRAW);
            //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            //glEnableVertexAttribArray(0);

            // remove temporary coordinates
        }
    }
}

void insertCoordinates(float xpos, float ypos, bool temporary) {
    /*
    glm::vec4 vec(xpos, ypos, 0.0f, 1.0f);
    glm::mat4 trans = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT, 0.1f, 10.0f);
    vec = vec * trans;
    coordinates.push_back(vec.x);
    coordinates.push_back(vec.y);
    coordinates.push_back(vec.z);
    */
    if (drawMode == DrawMode::line) {
        linesCoordinates.push_back((2.0f / (float)SCR_WIDTH) * xpos - 1);
        // taking the negative of the coordinate to flip Y-axis
        linesCoordinates.push_back(-((2.0f / (float)SCR_HEIGHT) * ypos - 1));
        linesCoordinates.push_back(0.0f);
    }
    else if (drawMode == DrawMode::polygon) {
        float xValue = (2.0f / (float)SCR_WIDTH) * xpos - 1;
        float yValue = -((2.0f / (float)SCR_HEIGHT) * ypos - 1);

        if (!temporary) {
            // check if inserted point is in vicinity of first point also check this only if at least one point is present in polygon
            if (polygonCoordinates.size() > polygonIndexes.back() && abs(xValue - polygonCoordinates[polygonIndexes.back()]) < 0.05f && abs(yValue - polygonCoordinates[polygonIndexes.back() + 1]) < 0.05f) {
                polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back()]);
                // taking the negative of the coordinate to flip Y-axis
                polygonCoordinates.push_back(polygonCoordinates[polygonIndexes.back() + 1]);
                polygonCoordinates.push_back(0.0f);
                polygonIndexes.push_back(polygonCoordinates.size());
            }
            else if (polygonCoordinates.size() > polygonIndexes.back()) {
                // add previous
                if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    // taking the negative of the coordinate to flip Y-axis
                    polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
                    polygonCoordinates.push_back(0.0f);
                }
                polygonCoordinates.push_back(xValue);
                // taking the negative of the coordinate to flip Y-axis
                polygonCoordinates.push_back(yValue);
                polygonCoordinates.push_back(0.0f);
            }
            else {
                polygonCoordinates.push_back(xValue);
                // taking the negative of the coordinate to flip Y-axis
                polygonCoordinates.push_back(yValue);
                polygonCoordinates.push_back(0.0f);
            }
        }
        else {
            polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
            // taking the negative of the coordinate to flip Y-axis
            polygonCoordinates.push_back(polygonCoordinates[polygonCoordinates.size() - 3]);
            polygonCoordinates.push_back(0.0f);
            polygonCoordinates.push_back(xValue);
            // taking the negative of the coordinate to flip Y-axis
            polygonCoordinates.push_back(yValue);
            polygonCoordinates.push_back(0.0f);
        }
        int j = 0;
        for (auto i = polygonCoordinates.begin(); i != polygonCoordinates.end(); ++i, ++j) {
            if (j % 3 == 0) printf("\n");
            printf("%f ", *i);
        }
        printf("\n");
        if (polygonCoordinates.size() > polygonIndexes.back() + 3) {
            float *vertices = &polygonCoordinates[0];
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * polygonCoordinates.size(), vertices, GL_DYNAMIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
        if (temporary) {
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
            polygonCoordinates.pop_back();
        }
    }
}
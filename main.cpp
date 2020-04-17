#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H 
#include "shaders.h"
#include "definitions.h"

// global variables
unsigned int SCR_WIDTH = 700;
unsigned int SCR_HEIGHT = 700;

DrawMode drawMode = DrawMode::none;
Transformation transformation = Transformation::none;

std::vector<float> menuBoxCoordinates;
std::vector<float> linesCoordinates;
std::vector<float> polygonCoordinates;
std::vector<int> polygonIndexes = { 0 };
std::vector<std::vector<float> *> filledPolygonCoordinates;
std::vector<float> transformationWindowCoordinates;
std::vector<char> keyboardInput1 = { '\0' };
std::vector<char> keyboardInput2 = { '\0' };

std::string tempString = "";

std::vector<unsigned int> VBO;
std::vector<unsigned int> VAO;
unsigned int vertexShader, fragmentShader, shaderProgram, textVertexShader, textFragmentShader, textShaderProgram;

std::map<GLchar, Character> characters;

bool listenForKeyboardInput = false;
bool spaced = false;
bool backSpaced = false;

GLFWcursor *crossHairCursor, *defaultCursor, *pointerCursor;

int main() {
    // GLFW initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // cursor
    crossHairCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    pointerCursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    defaultCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    glfwSetCursor(window, crossHairCursor);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);

    // keyboard
    glfwSetCharCallback(window, characterCallback);

    // initialize vertex buffer object
    VBO.assign({ 0, 0, 0, 0, 0 });
    for (int i = 0; i < VBO.size(); ++i) {
        glGenBuffers(1, &VBO[i]);
    }

    // initialize vertex array object
    VAO.assign({ 0, 0, 0, 0, 0 });
    for (int i = 0; i < VAO.size(); ++i) {
        glGenVertexArrays(1, &VAO[i]);
    }

    // initialize vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    textVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(textVertexShader, 1, &textVertexShaderSource, NULL);
    glCompileShader(textVertexShader);

    // initialize fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    textFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(textFragmentShader, 1, &textFragmentShaderSource, NULL);
    glCompileShader(textFragmentShader);

    // initialize shader program
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, textVertexShader);
    glAttachShader(textShaderProgram, textFragmentShader);
    glLinkProgram(textShaderProgram);
    glDeleteShader(textVertexShader);
    glDeleteShader(textFragmentShader);

    // bind VAO and VBO
    for (int i = 0; i < VAO.size(); ++i) {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        if (i == 3) { // different for text VBO
            glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
        }
        else {
            glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
        }
        glEnableVertexAttribArray(0);
        if (i == 3) { // different for text VAO
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
        }
        else {
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        }
    }

    // text
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
    }
    FT_Face face;
    if (FT_New_Face(ft, "fonts/Roboto/Roboto-Regular.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
    }
    // set font size
    FT_Set_Pixel_Sizes(face, 0, 48);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (GLubyte c = 0; c < 128; c++)
    {
        // load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            face->glyph->advance.x
        };
        characters.insert(std::pair<GLchar, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    // destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // initialize box coordinates
    menuBoxCoordinates.assign({
        // menu box
        -0.95f, 0.95f, 0.0f,
        -0.125f, 0.95f, 0.0f,
        -0.125f, 0.95f, 0.0f,
        -0.125f, 0.425f, 0.0f,
        -0.125f, 0.425f, 0.0f,
        -0.95f, 0.425f, 0.0f,
        -0.95f, 0.425f, 0.0f,
        -0.95f, 0.95f, 0.0f,
        // menu lines 1
        -0.95f, 0.885f, 0.0f,
        -0.125f, 0.885f, 0.0f,
        // menu lines 2
        -0.65f, 0.885f, 0.0f,
        -0.65f, 0.425f, 0.0f,
        // menu lines 3
        -0.95f, 0.825f, 0.0f,
        -0.125f, 0.825f, 0.0f,
        // draw button 1
        -0.90f, 0.8f, 0.0f,
        -0.70f, 0.8f, 0.0f,
        -0.70f, 0.8f, 0.0f,
        -0.70f, 0.75f, 0.0f,
        -0.70f, 0.75f, 0.0f,
        -0.90f, 0.75f, 0.0f,
        -0.90f, 0.75f, 0.0f,
        -0.90f, 0.8f, 0.0f,
        // draw button 2
        -0.90f, 0.725f, 0.0f,
        -0.70f, 0.725f, 0.0f,
        -0.70f, 0.725f, 0.0f,
        -0.70f, 0.675f, 0.0f,
        -0.70f, 0.675f, 0.0f,
        -0.90f, 0.675f, 0.0f,
        -0.90f, 0.675f, 0.0f,
        -0.90f, 0.725f, 0.0f,
        // draw button 3
        -0.90f, 0.65f, 0.0f,
        -0.70f, 0.65f, 0.0f,
        -0.70f, 0.65f, 0.0f,
        -0.70f, 0.60f, 0.0f,
        -0.70f, 0.60f, 0.0f,
        -0.90f, 0.60f, 0.0f,
        -0.90f, 0.60f, 0.0f,
        -0.90f, 0.65f, 0.0f,
        // draw button 4
        -0.90f, 0.575f, 0.0f,
        -0.70f, 0.575f, 0.0f,
        -0.70f, 0.575f, 0.0f,
        -0.70f, 0.525f, 0.0f,
        -0.70f, 0.525f, 0.0f,
        -0.90f, 0.525f, 0.0f,
        -0.90f, 0.525f, 0.0f,
        -0.90f, 0.575f, 0.0f,
        // transform button 1 A
        -0.60f, 0.8f, 0.0f,
        -0.40f, 0.8f, 0.0f,
        -0.40f, 0.8f, 0.0f,
        -0.40f, 0.75f, 0.0f,
        -0.40f, 0.75f, 0.0f,
        -0.60f, 0.75f, 0.0f,
        -0.60f, 0.75f, 0.0f,
        -0.60f, 0.8f, 0.0f,
        // transform button 1 B
        -0.375f, 0.8f, 0.0f,
        -0.175f, 0.8f, 0.0f,
        -0.175f, 0.8f, 0.0f,
        -0.175f, 0.75f, 0.0f,
        -0.175f, 0.75f, 0.0f,
        -0.375f, 0.75f, 0.0f,
        -0.375f, 0.75f, 0.0f,
        -0.375f, 0.8f, 0.0f,
        // transform button 2 A
        -0.60f, 0.725f, 0.0f,
        -0.40f, 0.725f, 0.0f,
        -0.40f, 0.725f, 0.0f,
        -0.40f, 0.675f, 0.0f,
        -0.40f, 0.675f, 0.0f,
        -0.60f, 0.675f, 0.0f,
        -0.60f, 0.675f, 0.0f,
        -0.60f, 0.725f, 0.0f,
        // transform button 2 B
        -0.375f, 0.725f, 0.0f,
        -0.175f, 0.725f, 0.0f,
        -0.175f, 0.725f, 0.0f,
        -0.175f, 0.675f, 0.0f,
        -0.175f, 0.675f, 0.0f,
        -0.375f, 0.675f, 0.0f,
        -0.375f, 0.675f, 0.0f,
        -0.375f, 0.725f, 0.0f,
        // transform button 3 A
        -0.60f, 0.65f, 0.0f,
        -0.40f, 0.65f, 0.0f,
        -0.40f, 0.65f, 0.0f,
        -0.40f, 0.60f, 0.0f,
        -0.40f, 0.60f, 0.0f,
        -0.60f, 0.60f, 0.0f,
        -0.60f, 0.60f, 0.0f,
        -0.60f, 0.65f, 0.0f,
        // transform button 3 B
        -0.375f, 0.65f, 0.0f,
        -0.175f, 0.65f, 0.0f,
        -0.175f, 0.65f, 0.0f,
        -0.175f, 0.60f, 0.0f,
        -0.175f, 0.60f, 0.0f,
        -0.375f, 0.60f, 0.0f,
        -0.375f, 0.60f, 0.0f,
        -0.375f, 0.65f, 0.0f,
        // transform button 4 A
        -0.60f, 0.575f, 0.0f,
        -0.40f, 0.575f, 0.0f,
        -0.40f, 0.575f, 0.0f,
        -0.40f, 0.525f, 0.0f,
        -0.40f, 0.525f, 0.0f,
        -0.60f, 0.525f, 0.0f,
        -0.60f, 0.525f, 0.0f,
        -0.60f, 0.575f, 0.0f,
        // transform button 4 B
        -0.375f, 0.575f, 0.0f,
        -0.175f, 0.575f, 0.0f,
        -0.175f, 0.575f, 0.0f,
        -0.175f, 0.525f, 0.0f,
        -0.175f, 0.525f, 0.0f,
        -0.375f, 0.525f, 0.0f,
        -0.375f, 0.525f, 0.0f,
        -0.375f, 0.575f, 0.0f,
        // transform button 5 AB
        -0.49f, 0.50f, 0.0f,
        -0.29f, 0.50f, 0.0f,
        -0.29f, 0.50f, 0.0f,
        -0.29f, 0.45f, 0.0f,
        -0.29f, 0.45f, 0.0f,
        -0.49f, 0.45f, 0.0f,
        -0.49f, 0.45f, 0.0f,
        -0.49f, 0.50f, 0.0f
    });

    // box buffer data
    glBindBuffer(GL_ARRAY_BUFFER, VBO[4]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * menuBoxCoordinates.size(), &menuBoxCoordinates[0], GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window)) {
        // process keyboard input
        processKeyboardInput(window);

        //render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // headings
        renderText("Menu", -0.59f, 0.90f, 0.75f, glm::vec3(0.1484375f, 0.20703125f, 0.828125f));
        renderText("Draw", -0.85f, 0.84f, 0.6f, glm::vec3(0.1484375f, 0.20703125f, 0.828125f));
        renderText("Transform", -0.48f, 0.84f, 0.6f, glm::vec3(0.1484375f, 0.20703125f, 0.828125f));
        // draw menu
        renderText("Line", -0.83f, 0.7625f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Polygon", -0.86f, 0.69f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Flood Fill", -0.87f, 0.615f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Clear", -0.84f, 0.54f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        // transform menu
        renderText("Translate", -0.579f, 0.7625f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Rotate", -0.33f, 0.7625f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Reflect X", -0.573f, 0.687f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Reflect Y", -0.343f, 0.687f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Reflect Origin", -0.595f, 0.615f, 0.45f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Scale", -0.32f, 0.615f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("X-Shear", -0.566f, 0.54f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Y-Shear", -0.339f, 0.54f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));
        renderText("Cancel", -0.442f, 0.465f, 0.525f, glm::vec3(0.0f, 0.0f, 0.0f));

        if (listenForKeyboardInput) {
            std::string firstString = keyboardInput1.data();
            std::string secondString = keyboardInput2.data();
            if (transformation == Transformation::translation || transformation == Transformation::scaling) {
                tempString = "Enter transformation factors: [X: " + firstString + ", Y: " + secondString + "]";
            }
            else {
                tempString = "Enter transformation factor: " + firstString;
            }
            renderText(tempString, -0.1f, 0.9f, 0.75f, glm::vec3(0.0f, 0.0f, 0.0f));
        }

        // draw
        glUseProgram(shaderProgram);
        int i = 0;
        glBindVertexArray(VAO[i++]);
        glDrawArrays(GL_LINES, 0, (linesCoordinates.size() / 3) + 1);
        glBindVertexArray(VAO[i++]);
        glDrawArrays(GL_LINES, 0, (polygonCoordinates.size() / 3) + 2);
        glBindVertexArray(VAO[i++]);
        glDrawArrays(GL_LINE_LOOP, 0, 4);

        // again for text (happens in a different function)
        i++;

        glBindVertexArray(VAO[i++]);
        glDrawArrays(GL_LINES, 0, menuBoxCoordinates.size() / 3);

        for (int j = i; j < VAO.size(); ++j) {
            glBindVertexArray(VAO[j]);
            glDrawArrays(GL_TRIANGLE_FAN, 0, filledPolygonCoordinates[j - i]->size() / 3);
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
    else if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
        if (listenForKeyboardInput) {
            if (transformation == Transformation::translation ||
                transformation == Transformation::scaling ||
                transformation == Transformation::rotation ||
                transformation == Transformation::shearX ||
                transformation == Transformation::shearY) {
                char* firstString = keyboardInput1.data();
                char* secondString = keyboardInput2.data();
                float x = (float)strtod(firstString, (char**)NULL);
                float y = (float)strtod(secondString, (char**)NULL);
                processTransformation(x, y);
            }
            listenForKeyboardInput = false;
            clearCharacterBuffer();
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
        if (!backSpaced && listenForKeyboardInput) {
            if (keyboardInput2.size() == 1) {
                spaced = false;
            }
            if (!spaced) {
                if (keyboardInput1.size() > 1) {
                    keyboardInput1.pop_back();
                    keyboardInput1.pop_back();
                    keyboardInput1.push_back('\0');
                }
            }
            else {
                keyboardInput2.pop_back();
                keyboardInput2.pop_back();
                keyboardInput2.push_back('\0');
            }
            backSpaced = true;
        }
    }
    else if (glfwGetKey(window, GLFW_KEY_BACKSPACE) == GLFW_RELEASE) {
        backSpaced = false;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float x = (float)xpos, y = (float)ypos;
    normalizeCoordinates(&x, &y);

    if (x > -0.125f || y < 0.425f || x < -0.95f || y > 0.95f) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            insertCoordinates((float)xpos, (float)ypos);
        }
    }
    else if (x >= -0.90f && x <= -0.70f && y >= 0.75f && y <= 0.80f) {
        refreshBuffer();
        drawMode = DrawMode::line;
    }
    else if (x >= -0.90f && x <= -0.70f && y >= 0.675f && y <= 0.725f) {
        refreshBuffer();
        drawMode = DrawMode::polygon;
    }
    else if (x >= -0.90f && x <= -0.70f && y >= 0.60f && y <= 0.65f) {
        refreshBuffer();
        drawMode = DrawMode::floodFill;
    }
    else if (x >= -0.90f && x <= -0.70f && y >= 0.525f && y <= 0.575f) {
        transformation = Transformation::none;
        drawMode = DrawMode::none;
        clearCoordinates();
    }
    else if (x >= -0.60f && x <= -0.40f && y >= 0.75f && y <= 0.80f) {
        refreshBuffer();
        transformation = Transformation::translation;
    }
    else if (x >= -0.375f && x <= -0.175f && y >= 0.75f && y <= 0.80f) {
        refreshBuffer();
        transformation = Transformation::rotation;
    }
    else if (x >= -0.60f && x <= -0.40f && y >= 0.675f && y <= 0.725f) {
        refreshBuffer();
        transformation = Transformation::reflectionX;
    }
    else if (x >= -0.375f && x <= -0.175f && y >= 0.675f && y <= 0.725f) {
        refreshBuffer();
        transformation = Transformation::reflectionY;
    }
    else if (x >= -0.60f && x <= -0.40f && y >= 0.60f && y <= 0.65f) {
        refreshBuffer();
        transformation = Transformation::reflectionOrigin;
    }
    else if (x >= -0.375f && x <= -0.175f && y >= 0.60f && y <= 0.65f) {
        refreshBuffer();
        transformation = Transformation::scaling;
    }
    else if (x >= -0.60f && x <= -0.40f && y >= 0.525f && y <= 0.575f) {
        refreshBuffer();
        transformation = Transformation::shearX;
    }
    else if (x >= -0.375f && x <= -0.175f && y >= 0.525f && y <= 0.575f) {
        refreshBuffer();
        transformation = Transformation::shearY;
    }
    else if (x >= -0.49f && x <= -0.29f && y >= 0.45f && y <= 0.50f) {
        transformation = Transformation::none;
        transformationWindowCoordinates.clear();
        refreshBuffer();
    }
}

void clearCoordinates() {
    linesCoordinates.clear();
    polygonCoordinates.clear();
    polygonIndexes.clear();
    polygonIndexes.push_back(0);
    for (int i = 0; i < filledPolygonCoordinates.size(); ++i) {
        filledPolygonCoordinates[i]->clear();
        delete filledPolygonCoordinates[i];
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i + 5]);
        glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
        glDeleteBuffers(1, &VAO[i + 5]);
        glDeleteBuffers(1, &VBO[i + 5]);
    }
    for (int i = 0; i < filledPolygonCoordinates.size(); ++i) {
        VAO.pop_back();
        VBO.pop_back();
    }
    filledPolygonCoordinates.clear();
    transformationWindowCoordinates.clear();
    for (int i = 0; i < VBO.size(); ++i) {
        if (i != 3 && i != 4) {
            glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
            glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_STATIC_DRAW);
        }
    }
    listenForKeyboardInput = false;
    clearCharacterBuffer();
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
    listenForKeyboardInput = false;
    clearCharacterBuffer();
}

void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    float x = (float)xpos, y = (float)ypos;
    normalizeCoordinates(&x, &y);

    if ((x >= -0.49f && x <= -0.29f && y >= 0.45f && y <= 0.50f) || 
        (x >= -0.375f && x <= -0.175f && y >= 0.525f && y <= 0.575f) ||
        (x >= -0.60f && x <= -0.40f && y >= 0.525f && y <= 0.575f) ||
        (x >= -0.375f && x <= -0.175f && y >= 0.60f && y <= 0.65f) ||
        (x >= -0.60f && x <= -0.40f && y >= 0.60f && y <= 0.65f) ||
        (x >= -0.375f && x <= -0.175f && y >= 0.675f && y <= 0.725f) ||
        (x >= -0.60f && x <= -0.40f && y >= 0.675f && y <= 0.725f) ||
        (x >= -0.375f && x <= -0.175f && y >= 0.75f && y <= 0.80f) ||
        (x >= -0.60f && x <= -0.40f && y >= 0.75f && y <= 0.80f) ||
        (x >= -0.90f && x <= -0.70f && y >= 0.525f && y <= 0.575f) ||
        (x >= -0.90f && x <= -0.70f && y >= 0.60f && y <= 0.65f) ||
        (x >= -0.90f && x <= -0.70f && y >= 0.675f && y <= 0.725f) ||
        (x >= -0.90f && x <= -0.70f && y >= 0.75f && y <= 0.80f)) {
        glfwSetCursor(window, pointerCursor);
    }
    else if (x <= -0.125f && y >= 0.425f && x >= -0.95f && y <= 0.95f) {
        glfwSetCursor(window, defaultCursor);
    }
    else {
        glfwSetCursor(window, crossHairCursor);
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
        if ((char)codepoint == ' ') {
            spaced = true;
        }
        else if (!spaced) {
            keyboardInput1.insert(--keyboardInput1.end(), (char)codepoint);
        }
        else {
            keyboardInput2.insert(--keyboardInput2.end(), (char)codepoint);
        }
        
    }
}

void clearCharacterBuffer() {
    keyboardInput1.clear();
    keyboardInput2.clear();
    keyboardInput1.push_back('\0');
    keyboardInput2.push_back('\0');
    spaced = false;
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
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i + 5]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)* filledPolygonCoordinates[i]->size(), filledPolygonCoordinates[i]->data(), GL_STATIC_DRAW);
    }
}

void normalizeCoordinates(float *x, float *y) {
    *x = (2.0f / (float)SCR_WIDTH) * *x - 1;
    // flip Y coordinate
    *y = -((2.0f / (float)SCR_HEIGHT) * *y - 1);
}

void renderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {

    // convert x and y to coords
    x *= SCR_WIDTH;
    y *= SCR_HEIGHT;

    glUseProgram(textShaderProgram);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO[3]);

    // iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

        GLfloat w = ch.size.x * scale;
        GLfloat h = ch.size.y * scale;
        // update VBO for each character
        GLfloat vertices[6][4] = {
            { xpos / SCR_WIDTH,     (ypos + h) / SCR_HEIGHT,   0.0, 0.0 },
            { xpos / SCR_WIDTH,     ypos / SCR_HEIGHT,       0.0, 1.0 },
            { (xpos + w) / SCR_WIDTH, ypos / SCR_HEIGHT,       1.0, 1.0 },

            { xpos / SCR_WIDTH,     (ypos + h) / SCR_HEIGHT,   0.0, 0.0 },
            { (xpos + w) / SCR_WIDTH, ypos / SCR_HEIGHT,       1.0, 1.0 },
            { (xpos + w) / SCR_WIDTH, (ypos + h) / SCR_HEIGHT,   1.0, 0.0 }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO[3]);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6)* scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <rg/Texture2D.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
bool flashLight = false;
bool flashLightKeyPressed = false;

// camera
//Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
//Camera camera(glm::vec3(8.0f, 3.0f, 10.0f));
Camera camera(glm::vec3(14.0f, 14.0f, 12.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos = glm::vec3(0.0f, 10.0f, 0.0f);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 4);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    Shader floorShader("resources/shaders/cube.vs", "resources/shaders/cube.fs");
    Shader blendingShader("resources/shaders/blendingShader.vs", "resources/shaders/blendingShader.fs");
    Shader pyramidShader("resources/shaders/pyramid.vs", "resources/shaders/pyramid.fs");
    Shader objectShader("resources/shaders/objectShader.vs", "resources/shaders/objectShader.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");

//    stbi_set_flip_vertically_on_load(true);

    Model sphere(FileSystem::getPath("resources/objects/xxr-sphere/XXR_B_BLOODSTONE_002.obj"));
    sphere.SetShaderTextureNamePrefix("material.");

    Shader tableTopCubeShader("resources/shaders/tableTopCube.vs", "resources/shaders/tableTopCube.fs");

    float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
            1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
            1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
            1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
    };

    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindVertexArray(cubeVAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // pyramid coordinates
    float pyramidVertices[] = {
            // positions         // normals           // texture coords
            1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,    //A
            0.0f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,  5.0f,  0.0f,    //B
            -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,    //C
            0.0f, -1.0f,  0.0f,  0.0f, -1.0f,  0.0f,  5.0f,  0.0f,    //D
            0.0f,  0.0f,  2.0f,  0.0f,  0.0f,  1.0f,  2.5f,  5.0f,	   //E
    };

    unsigned int pyramidIndices[] = {
            0, 3, 1,
            1, 3, 2,
            0, 1, 4,
            0, 4, 3,
            2, 3, 4,
            1, 2, 4
    };

    unsigned int pyramidVAO, pyramidVBO, pyramidEBO;
    glGenVertexArrays(1, &pyramidVAO);
    glGenBuffers(1, &pyramidVBO);
    glGenBuffers(1, &pyramidEBO);

    glBindVertexArray(pyramidVAO);
    glBindBuffer(GL_ARRAY_BUFFER, pyramidVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramidVertices), pyramidVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pyramidEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(pyramidIndices), pyramidIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    Texture2D woodTexture("resources/textures/table.jpg", 0);
    Texture2D pyramidTexture("resources/textures/bricks2.jpg", 1);

    floorShader.use();
    floorShader.setInt("material.specular", woodTexture.getTextureNumber());

    pyramidShader.use();
    pyramidShader.setInt("material.diffuse", pyramidTexture.getTextureNumber());

    // tabletop cube definitions and light

    unsigned int tableTopCubeVBO, tableTopCubeVAO;
    glGenVertexArrays(1, &tableTopCubeVAO);
    glGenBuffers(1, &tableTopCubeVBO);

    glBindBuffer(GL_ARRAY_BUFFER, tableTopCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(tableTopCubeVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    Texture2D tableTopCubeTexture("resources/textures/red_brick3.jpg", 2);

    tableTopCubeShader.use();
    tableTopCubeShader.setInt("material.diffuse", tableTopCubeTexture.getTextureNumber());

    // transparent vertices

    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f
    };

    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);

    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);

    glBindVertexArray(transparentVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    Texture2D transparentTexture("resources/textures/crack.png", 3);

    blendingShader.use();
    blendingShader.setInt("texture1", transparentTexture.getTextureNumber());

    // light source cube

    Shader lightCubeShader("resources/shaders/lightcube.vs", "resources/shaders/lightcube.fs");

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // skybox setup

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    vector<std::string> skyboxSides = {
            FileSystem::getPath("resources/textures/skyboxTextures/right.png"), // 0
            FileSystem::getPath("resources/textures/skyboxTextures/left.png"),  // 1
            FileSystem::getPath("resources/textures/skyboxTextures/top.png"),  // 2
            FileSystem::getPath("resources/textures/skyboxTextures/bottom.png"),  // 3
            FileSystem::getPath("resources/textures/skyboxTextures/front.png"),  // 4
            FileSystem::getPath("resources/textures/skyboxTextures/back.png") // 5
    };

    Texture2D skyboxTexture(skyboxSides, 4);

    skyboxShader.use();
    skyboxShader.setInt("skybox", skyboxTexture.getTextureNumber());

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // spinning cube

        lightPos.x = 3*sin(currentFrame)+1;
        lightPos.z = 3*cos(currentFrame)+1;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        woodTexture.bind();
        pyramidTexture.bind();
        tableTopCubeTexture.bind();
        transparentTexture.bind();
        skyboxTexture.bindCubemap();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // Floor setup.
        floorShader.use();
        floorShader.setVec3("viewPos", lightPos);
        floorShader.setFloat("material.shininess", 7.0f);
        floorShader.setInt("flashLight", flashLight); // da ili ne?

        // light properties

        // TODO: isto kao kod tableTopCube problem

        floorShader.setVec3("dirLight.direction", 9.0f, 2.1f, 9.0f);
        floorShader.setVec3("dirLight.ambient", glm::vec3(0.2));
        floorShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        floorShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        floorShader.setVec3("pointLight.position", lightPos);
        floorShader.setVec3("pointLight.ambient", 0.1f, 0.1f, 0.1f);
        floorShader.setVec3("pointLight.diffuse", 0.95f, 0.95f, 0.95f);
        floorShader.setVec3("pointLight.specular", 0.5f, 0.5f, 0.5f);
        floorShader.setFloat("pointLight.constant", 1.0f);
        floorShader.setFloat("pointLight.linear", 0.22f);
        floorShader.setFloat("pointLight.quadratic", 0.0009f);

        floorShader.setVec3("spotLight.position", camera.Position);
        floorShader.setVec3("spotLight.direction", camera.Front);
        floorShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        floorShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        floorShader.setVec3("spotLight.specular", 0.8f, 0.8f, 0.8f);
        floorShader.setFloat("spotLight.constant", 1.0f);
        floorShader.setFloat("spotLight.linear", 0.007f);
        floorShader.setFloat("spotLight.quadratic", 0.0002f);
        floorShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        floorShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        floorShader.setMat4("projection", projection);
        floorShader.setMat4("view", view);
        model = glm::scale(model, glm::vec3(12.5f, 0.1f, 12.5f));
        floorShader.setMat4("model", model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Edges of the table.
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, 12.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(12.5f, 0.1f, 1.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, -12.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(12.5f, 0.1f, 1.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(12.5f, 0, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(12.5f, 0.1f, 1.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-12.5f, 0, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1, 0, 0));
        model = glm::scale(model, glm::vec3(12.5f, 0.1f, 1.0f));
        floorShader.setMat4("model", model);
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // transparent setup

        blendingShader.use();
        model = glm::mat4(1.0f);
        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);
        model = glm::translate(model, glm::vec3(6.8f, 2.4f, 9.0f));
        model = glm::rotate(model, glm::radians(-70.0f), glm::vec3(0.0f, -1.0, 0.0f));
        model = glm::scale(model, glm::vec3(2.7f));

        blendingShader.setMat4("model", model);
        glBindVertexArray(transparentVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Pyramid setup.

        pyramidShader.use();
        pyramidShader.setVec3("viewPos", lightPos);
        pyramidShader.setFloat("material.shininess", 1.0f);
        pyramidShader.setInt("flashLight", flashLight);

        // light properties

        // TODO: sta cemo sa direkcionim tj odakle? najverovatnije ce to biti kada dodamo cubemaps?

        pyramidShader.setVec3("dirLight.direction", -9, 0.1f, 8.5f);
        pyramidShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        pyramidShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        pyramidShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        pyramidShader.setVec3("pointLight.position", lightPos);
        pyramidShader.setVec3("pointLight.ambient", 0.1f, 0.1f, 0.05f);
        pyramidShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        pyramidShader.setVec3("pointLight.specular", 0.0f, 0.0f, 0.0f);
        pyramidShader.setFloat("pointLight.constant", 1.0f);
        pyramidShader.setFloat("pointLight.linear", 0.07f);
        pyramidShader.setFloat("pointLight.quadratic", 0.00002f);

        pyramidShader.setVec3("spotLight.position", camera.Position);
        pyramidShader.setVec3("spotLight.direction", camera.Front);
        pyramidShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        pyramidShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        pyramidShader.setVec3("spotLight.specular", 0.8f, 0.8f, 0.8f);
        pyramidShader.setFloat("spotLight.constant", 1.0f);
        pyramidShader.setFloat("spotLight.linear", 0.007f);
        pyramidShader.setFloat("spotLight.quadratic", 0.0002f);
        pyramidShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        pyramidShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-9, 0.1f, 8.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0, 0.0f));
        model = glm::scale(model, glm::vec3(3.0f));
        pyramidShader.setMat4("model", model);
        pyramidShader.setMat4("projection", projection);
        pyramidShader.setMat4("view", view);

        glBindVertexArray(pyramidVAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-9, 0.1f, 3.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0, 0.0f));
        model = glm::scale(model, glm::vec3(2.5f));
        pyramidShader.setMat4("model", model);

        glBindVertexArray(pyramidVAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.7, 0.1f, 5.3));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        pyramidShader.setMat4("model", model);

        glBindVertexArray(pyramidVAO);
        glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);

        // Table top cubes

        tableTopCubeShader.use();
        tableTopCubeShader.setVec3("viewPos", lightPos);
        tableTopCubeShader.setFloat("material.shininess", 32.0f);
        tableTopCubeShader.setInt("flashLight", flashLight);

        // light properties

        // TODO: sta cemo sa direkcionim tj odakle? najverovatnije ce to biti kada dodamo cubemaps?

        tableTopCubeShader.setVec3("dirLight.direction", 9.0f, 2.1f, 9.0f);
        tableTopCubeShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        tableTopCubeShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        tableTopCubeShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        tableTopCubeShader.setVec3("pointLight.position", lightPos);
        tableTopCubeShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
        tableTopCubeShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        tableTopCubeShader.setVec3("pointLight.specular", 0.0f, 0.0f, 0.0f);
        tableTopCubeShader.setFloat("pointLight.constant", 1.0f);
        tableTopCubeShader.setFloat("pointLight.linear", 0.007f);
        tableTopCubeShader.setFloat("pointLight.quadratic", 0.0002f);

        tableTopCubeShader.setVec3("spotLight.position", camera.Position);
        tableTopCubeShader.setVec3("spotLight.direction", camera.Front);
        tableTopCubeShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        tableTopCubeShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        tableTopCubeShader.setVec3("spotLight.specular", 1.2f, 1.2f, 1.2f);
        tableTopCubeShader.setFloat("spotLight.constant", 1.0f);
        tableTopCubeShader.setFloat("spotLight.linear", 0.007f);
        tableTopCubeShader.setFloat("spotLight.quadratic", 0.0002f);
        tableTopCubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        tableTopCubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // cube 1

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.0f, 2.1f, 9.0f));
        model = glm::rotate(model, glm::radians(-20.0f), glm::vec3(0.0, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(2.0f));

        tableTopCubeShader.setMat4("model", model);
        tableTopCubeShader.setMat4("view", view);
        tableTopCubeShader.setMat4("projection", projection);

        glBindVertexArray(tableTopCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // cube 2

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(9.0f, 1.6f, 3.0f));
        model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.5f));
        tableTopCubeShader.setMat4("model", model);
        tableTopCubeShader.setMat4("view", view);
        tableTopCubeShader.setMat4("projection", projection);

        glBindVertexArray(tableTopCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // cube 3

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.5f, 1.1f, 6.0f));
        tableTopCubeShader.setMat4("model", model);
        tableTopCubeShader.setMat4("view", view);
        tableTopCubeShader.setMat4("projection", projection);

        glBindVertexArray(tableTopCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Model setup.

        objectShader.use();
        objectShader.setVec3("viewPos", lightPos);
        objectShader.setFloat("material.shininess", 18.0f);
        objectShader.setInt("flashLight", flashLight);

        // light properties

        // TODO: sta cemo sa direkcionim tj odakle? najverovatnije ce to biti kada dodamo cubemaps?

        // takodje komentarisan deo za dirLight jer pravi 2 mala kruga na poledjini sfere
        // i u objectShader.fs je ovaj deo koji racuna dirLight zakomentarisan u main-u

        objectShader.setVec3("dirLight.direction", 9.0f, 2.1f, 9.0f);
        objectShader.setVec3("dirLight.ambient", 0.01f, 0.01f, 0.01f);
        objectShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
        objectShader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);

        objectShader.setVec3("pointLight.position", lightPos);
        objectShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
        objectShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        objectShader.setVec3("pointLight.specular", 0.0f, 0.0f, 0.0f);
        objectShader.setFloat("pointLight.constant", 1.0f);
        objectShader.setFloat("pointLight.linear", 0.007f);
        objectShader.setFloat("pointLight.quadratic", 0.0002f);

        objectShader.setVec3("spotLight.position", camera.Position);
        objectShader.setVec3("spotLight.direction", camera.Front);
        objectShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        objectShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        objectShader.setVec3("spotLight.specular", 1.2f, 1.2f, 1.2f);
        objectShader.setFloat("spotLight.constant", 1.0f);
        objectShader.setFloat("spotLight.linear", 0.007f);
        objectShader.setFloat("spotLight.quadratic", 0.0002f);
        objectShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        objectShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-7.0f, -1.0f, -2.0f));
        model = glm::scale(model, glm::vec3(1.8f));
        objectShader.setMat4("model", model);
        sphere.Draw(objectShader);

        // Lighting cube defining

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(1.5f));
        lightCubeShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // skybox

        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render skybox cube
        glBindVertexArray(skyboxVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &pyramidVAO);

    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &pyramidVBO);
    glDeleteBuffers(1, &pyramidEBO);

    glDeleteVertexArrays(1, &tableTopCubeVAO);
    glDeleteBuffers(1, &tableTopCubeVBO);
    glDeleteVertexArrays(1, &lightCubeVAO);

    glDeleteBuffers(1, &transparentVBO);
    glDeleteVertexArrays(1, &transparentVAO);

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    // Position of the camera.
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        std::cerr << "(" << camera.Position.x << "," << camera.Position.y << "," << camera.Position.z << ")\n";

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.MovementSpeed += 0.5f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.MovementSpeed -= 0.5f;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashLightKeyPressed)
    {
        flashLight = !flashLight;
        flashLightKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        flashLightKeyPressed = false;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.01f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
}

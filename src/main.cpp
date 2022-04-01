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
glm::vec3 dirPos = glm::vec3(-40.0f, 10.0f, -40.0f);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //glfwWindowHint(GLFW_SAMPLES, 4);

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
    //glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    Shader floorShader("resources/shaders/cube.vs", "resources/shaders/cube.fs");
    Shader blendingShader("resources/shaders/blendingShader.vs", "resources/shaders/blendingShader.fs");
    Shader pyramidShader("resources/shaders/pyramid.vs", "resources/shaders/pyramid.fs");
    Shader objectShader("resources/shaders/objectShader.vs", "resources/shaders/objectShader.fs");
    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");
    Shader lightCubeShader("resources/shaders/lightcube.vs", "resources/shaders/lightcube.fs");

    // Deferred Shading
    Shader geometryPassShader("resources/shaders/gBuffer.vs", "resources/shaders/gBuffer.fs");
    Shader lightingPassShader("resources/shaders/deferredShading.vs", "resources/shaders/deferredShading.fs");

//    stbi_set_flip_vertically_on_load(true);

    Model sphere(FileSystem::getPath("resources/objects/xxr-sphere/XXR_B_BLOODSTONE_002.obj"));
    sphere.SetShaderTextureNamePrefix("material.");

    Shader tableTopCubeShader("resources/shaders/tableTopCube.vs", "resources/shaders/tableTopCube.fs");

//    Model backpack(FileSystem::getPath("resources/objects/backpack/backpack.obj"));
//    std::vector<glm::vec3> objectPositions;
//    objectPositions.push_back(glm::vec3(-3.0,  -0.5, -3.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -0.5, -3.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -0.5, -3.0));
//    objectPositions.push_back(glm::vec3(-3.0,  -0.5,  0.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -0.5,  0.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -0.5,  0.0));
//    objectPositions.push_back(glm::vec3(-3.0,  -0.5,  3.0));
//    objectPositions.push_back(glm::vec3( 0.0,  -0.5,  3.0));
//    objectPositions.push_back(glm::vec3( 3.0,  -0.5,  3.0));

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

    vector<std::string> skyboxSides = {
            FileSystem::getPath("resources/textures/skyboxTextures/right.jpg"), // 0
            FileSystem::getPath("resources/textures/skyboxTextures/left.jpg"),  // 1
            FileSystem::getPath("resources/textures/skyboxTextures/top.jpg"),  // 2
            FileSystem::getPath("resources/textures/skyboxTextures/bottom.jpg"),  // 3
            FileSystem::getPath("resources/textures/skyboxTextures/front.jpg"),  // 4
            FileSystem::getPath("resources/textures/skyboxTextures/back.jpg") // 5
    };

    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  0.0f
    };

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



    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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


    Texture2D woodTexture("resources/textures/table.jpg", 0);
    Texture2D pyramidTexture("resources/textures/bricks2.jpg", 1);
    Texture2D tableTopCubeTexture("resources/textures/red_brick3.jpg", 2);
    Texture2D transparentTexture("resources/textures/crack.png", 3);
    Texture2D skyboxTexture(skyboxSides, 4);

    skyboxShader.use();
    skyboxShader.setInt("skybox", skyboxTexture.getTextureNumber());

    floorShader.use();
    floorShader.setInt("material.specular", woodTexture.getTextureNumber());

    pyramidShader.use();
    pyramidShader.setInt("material.diffuse", pyramidTexture.getTextureNumber());


    tableTopCubeShader.use();
    tableTopCubeShader.setInt("material.diffuse", tableTopCubeTexture.getTextureNumber());


    blendingShader.use();
    blendingShader.setInt("texture1", transparentTexture.getTextureNumber());

    // light source cube


    // TODO: configure g-buffer framebuffer
    //-------------------------------------
    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gAlbedoSpec;

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // tell OpenGL which color attachment we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_RENDERBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // finaly check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    // zavrsili kreiranje framebuffer-a
    //---------------------------------



    // TODO: shader configuration
    //---------------------------
    lightingPassShader.use();
    lightingPassShader.setInt("gPosition", 0);
    lightingPassShader.setInt("gNormal", 1);
    lightingPassShader.setInt("gAlbedoSpec", 2);
    //---------------------------



    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        // 1. geometry pass: render scene's geometry/color data into gbuffer
        // -----------------------------------------------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        geometryPassShader.use();
        geometryPassShader.setMat4("projection", projection);
        geometryPassShader.setMat4("view", view);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-7.0f, -1.0f, -2.0f));
        model = glm::scale(model, glm::vec3(1.8f));
        geometryPassShader.setMat4("model", model);
        sphere.Draw(geometryPassShader);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // -----------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightingPassShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        // send light relevant uniforms
            lightingPassShader.setVec3("lights[0].Position", lightPos);
            lightingPassShader.setVec3("lights[0].Color", glm::vec3(1.0f));
            // update attenuation parameters and calculate radius
            const float linear = 0.7f;
            const float quadratic = 1.8f;
            lightingPassShader.setFloat("lights[0].Linear", linear);
            lightingPassShader.setFloat("lights[0].Quadratic", quadratic);
        lightingPassShader.setVec3("viewPos", camera.Position);
        // finally render quad
        lightingPassShader.setMat4("projection", projection);
        lightingPassShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(1.5f));
        lightingPassShader.setMat4("model", model);

        glBindVertexArray(lightCubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

//        renderQuad();

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
        // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
        // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
        // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. render lights on top of scene
        // --------------------------------
//        lightCubeShader.use();
//        lightCubeShader.setMat4("projection", projection);
//        lightCubeShader.setMat4("projection", projection);
//        for (unsigned int i = 0; i < lightPositions.size(); i++)
//        {
//            model = glm::mat4(1.0f);
//            model = glm::translate(model, lightPositions[i]);
//            model = glm::scale(model, glm::vec3(0.125f));
//            lightCubeShader.setMat4("model", model);
//            shaderLightBox.setVec3("lightColor", lightColors[i]);
//            renderCube();
//        }

        // spinning cube
        // the following two lines are commented out, but can be uncommented
        // if you wish to see how the constant changing of the light position
        // affects the lighting on objects

        lightPos.x = 5*sin(currentFrame)+1;
        lightPos.z = 5*cos(currentFrame)+1;

        processInput(window);

        woodTexture.bind();
        pyramidTexture.bind();
        tableTopCubeTexture.bind();
        transparentTexture.bind();
        skyboxTexture.bindCubemap();

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /*
        // TODO: 1. Geometry pass:
        //------------------------
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 model = glm::mat4(1.0f);

            geometryPassShader.use();
            geometryPassShader.setMat4("projection", projection);
            geometryPassShader.setMat4("view", view);
            geometryPassShader.setMat4("model", model);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(-7.0f, -1.0f, -2.0f));
            model = glm::scale(model, glm::vec3(1.8f));
            geometryPassShader.setMat4("model", model);
            sphere.Draw(geometryPassShader);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // TODO: 2. Light pass:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        lightingPassShader.use();
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

        // send light relevent uniforms
        lightingPassShader.setVec3("lights[0].Position", lightPos);
        lightingPassShader.setVec3("lights[0].Color", glm::vec3(1.0f));
        const float linear = 0.7;
        const float quadratic = 1.8;
        lightingPassShader.setFloat("lights[0].Linear", linear);
        lightingPassShader.setFloat("lights[0].Quadratic", quadratic);
        lightingPassShader.setVec3("viewPos", lightPos);

        // finally render quadz (izvor svetlosti??)

        // TODO: 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        //-----------------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // TODO: 3. Render everything else: froward rendering
         */



        // Floor setup.
        floorShader.use();
        floorShader.setVec3("viewPos", lightPos);
        floorShader.setFloat("material.shininess", 7.0f);
        floorShader.setInt("flashLight", flashLight);

        // light properties

        floorShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        floorShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        floorShader.setVec3("dirLight.diffuse", 0.55f, 0.55f, 0.55f);
        floorShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

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
        floorShader.setVec3("spotLight.diffuse", 0.5f, 0.5f, 0.5f);
        floorShader.setVec3("spotLight.specular", 0.03f, 0.03f, 0.03f);
        floorShader.setFloat("spotLight.constant", 1.0f);
        floorShader.setFloat("spotLight.linear", 0.007f);
        floorShader.setFloat("spotLight.quadratic", 0.0002f);
        floorShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(7.5f)));
        floorShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(13.0f)));

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

        pyramidShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        pyramidShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        pyramidShader.setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
        pyramidShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

        pyramidShader.setVec3("pointLight.position", lightPos);
        pyramidShader.setVec3("pointLight.ambient", 0.1f, 0.1f, 0.05f);
        pyramidShader.setVec3("pointLight.diffuse", 0.4f, 0.4f, 0.4f);
        pyramidShader.setVec3("pointLight.specular", 0.6f, 0.6f, 0.6f);
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

        // directional light comes from the window of the skybox
        // which is approximately (somewhere) behind the cubes, and is not as bright

        tableTopCubeShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        tableTopCubeShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        tableTopCubeShader.setVec3("dirLight.diffuse", 0.55f, 0.55f, 0.55f);
        tableTopCubeShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

        tableTopCubeShader.setVec3("pointLight.position", lightPos);
        tableTopCubeShader.setVec3("pointLight.ambient", 0.05f, 0.05f, 0.05f);
        tableTopCubeShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        tableTopCubeShader.setVec3("pointLight.specular", 0.3f, 0.3f, 0.3f);
        tableTopCubeShader.setFloat("pointLight.constant", 1.0f);
        tableTopCubeShader.setFloat("pointLight.linear", 0.007f);
        tableTopCubeShader.setFloat("pointLight.quadratic", 0.0002f);

        tableTopCubeShader.setVec3("spotLight.position", camera.Position);
        tableTopCubeShader.setVec3("spotLight.direction", camera.Front);
        tableTopCubeShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        tableTopCubeShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        tableTopCubeShader.setVec3("spotLight.specular", 0.3f, 0.3f, 0.3f);
        tableTopCubeShader.setFloat("spotLight.constant", 1.0f);
        tableTopCubeShader.setFloat("spotLight.linear", 0.007f);
        tableTopCubeShader.setFloat("spotLight.quadratic", 0.0002f);
        tableTopCubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(9.0f)));
        tableTopCubeShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.0f)));

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

        objectShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        objectShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        objectShader.setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
        objectShader.setVec3("dirLight.specular", 0.0f, 0.0f, 0.0f);

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



        // Lighting cube defining

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

//    glDeleteVertexArrays(1, &tableTopCubeVAO);
//    glDeleteBuffers(1, &tableTopCubeVBO);
//    glDeleteVertexArrays(1, &lightCubeVAO);
//
//    glDeleteBuffers(1, &transparentVBO);
//    glDeleteVertexArrays(1, &transparentVAO);
//
//    glDeleteVertexArrays(1, &skyboxVAO);
//    glDeleteBuffers(1, &skyboxVBO);

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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
}

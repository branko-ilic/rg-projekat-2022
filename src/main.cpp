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
void renderQuad();
unsigned int loadTexture(char const * path);

// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;
bool flashLight = false;
bool flashLightKeyPressed = false;
bool bloomKeyPressed = false;
bool bloom = false;
float exposure = 2.5f; // tweak this
bool cursorToggle = false;

// AABloom = true activates bloom
// AABloom = false activates anti aliasing
bool AABloom = true;
bool AABloomKeyPressed = false;

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
glm::vec3 dirPos = glm::vec3(60, 20, 60);
float heightScale = 0.1;

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Book shop", NULL, NULL);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    Shader floorShader("resources/shaders/uniformLightShader.vs", "resources/shaders/uniformLightShader.fs");
    Shader pyramidShader("resources/shaders/uniformLightShader.vs", "resources/shaders/uniformLightShader.fs");
    Shader objectShader("resources/shaders/uniformLightShader.vs", "resources/shaders/uniformLightShader.fs");
    Shader tableTopCubeShader("resources/shaders/uniformLightShader.vs", "resources/shaders/uniformLightShader.fs");
    Shader blendingShader("resources/shaders/blendingShader.vs", "resources/shaders/blendingShader.fs");

    Shader skyboxShader("resources/shaders/skyboxShader.vs", "resources/shaders/skyboxShader.fs");
    Shader lightCubeShader("resources/shaders/uniformLightShader.vs", "resources/shaders/lightcube.fs");

    Shader shaderBlur("resources/shaders/bloomShaders/blur.vs", "resources/shaders/bloomShaders/blur.fs");
    Shader shaderBloomFinal("resources/shaders/bloomShaders/bloom.vs", "resources/shaders/bloomShaders/bloom.fs");


    Shader plantShader("resources/shaders/plantShader.vs", "resources/shaders/plantShader.fs");
    Model plant(FileSystem::getPath("resources/objects/azalea/Azalea_SF.obj"), true);
    plant.SetShaderTextureNamePrefix("material.");

    Shader bookShader("resources/shaders/bookShader.vs", "resources/shaders/bookShader.fs");
    Model book(FileSystem::getPath("resources/objects/hobbit-book/hobbit_book_SF.obj"));
    book.SetShaderTextureNamePrefix("material.");

    Model sphere(FileSystem::getPath("resources/objects/xxr-sphere/XXR_B_BLOODSTONE_002.obj"), true);
    sphere.SetShaderTextureNamePrefix("material.");

    // Sejder za framebuffer - crta pravougaonik preko celog ekrana na koji ce
    // biti nalepljena renderovana slika scene (nakon postprocesiranja).
    Shader screenShader("resources/shaders/screenShader.vs", "resources/shaders/screenShader.fs");

    // configure MSAA framebuffer
    // --------------------------
    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    // create a multisampled color attachment texture
    unsigned int textureColorBufferMultiSampled;
    glGenTextures(1, &textureColorBufferMultiSampled);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB, SCR_WIDTH, SCR_HEIGHT, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, textureColorBufferMultiSampled, 0);
    // create a (also multisampled) renderbuffer object for depth and stencil attachments
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // configure second post-processing framebuffer
    unsigned int intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    // create a color attachment texture
    unsigned int screenTexture;
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	// we only need a color buffer

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //Bloom-------------------------------------------------------------------------------------------------------------
    // configure framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0); //<=
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);//<=
    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }

    pyramidShader.use();
    pyramidShader.setInt("diffuseTexture", 0);
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);

    // cube vertices

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

    float quadVertices[] = {   // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
            // positions   // texCoords
            -1.0f,  1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
            1.0f, -1.0f,  1.0f, 0.0f,

            -1.0f,  1.0f,  0.0f, 1.0f,
            1.0f, -1.0f,  1.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f
    };
    // setup screen VAO
    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

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
            FileSystem::getPath("resources/textures/skyboxTextures/right.jpg"), // 0
            FileSystem::getPath("resources/textures/skyboxTextures/left.jpg"),  // 1
            FileSystem::getPath("resources/textures/skyboxTextures/top.jpg"),  // 2
            FileSystem::getPath("resources/textures/skyboxTextures/bottom.jpg"),  // 3
            FileSystem::getPath("resources/textures/skyboxTextures/front.jpg"),  // 4
            FileSystem::getPath("resources/textures/skyboxTextures/back.jpg") // 5
    };

    Texture2D skyboxTexture(skyboxSides, 4);

    skyboxShader.use();
    skyboxShader.setInt("skybox", skyboxTexture.getTextureNumber());

    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    // TODO: Da li moze preko klase Texture2D
    unsigned int heightMap = loadTexture("resources/objects/hobbit-book/hobbit_book_retopo_height.jpg");

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // spinning cube
        // the following two lines are commented out, but can be uncommented
        // if you wish to see how the constant changing of the light position
        // affects the lighting on objects

//        lightPos.x = 5*sin(currentFrame)+1;
//        lightPos.z = 5*cos(currentFrame)+1;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render scene into floating point framebuffer
        // -----------------------------------------------BLOOM

        if(AABloom){
            glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
        }else{
            // 1. draw scene as normal in multisampled buffers
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
        }

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

        // Models setup.

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

        // sphere model

        objectShader.setMat4("projection", projection);
        objectShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(8.0f, -1.6f, -7.5f));
        model = glm::scale(model, glm::vec3(2.8f));
        objectShader.setMat4("model", model);
        sphere.Draw(objectShader);

        // Plant model with normal mapping.

        plantShader.use();
        plantShader.setMat4("projection", projection);
        plantShader.setMat4("view", view);

        plantShader.setVec3("lightPos", lightPos);
        plantShader.setVec3("viewPos", lightPos);
        plantShader.setVec3("lightDir", dirPos);
        plantShader.setInt("flashLight", flashLight);
        plantShader.setFloat("material.shininess", 18.0f);
        // Salju se vertex shader-u
        plantShader.setVec3("spotPosition", camera.Position);
        plantShader.setVec3("spotDirection", camera.Front);

        plantShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        plantShader.setVec3("dirLight.ambient", 0.1f, 0.1f, 0.1f);
        plantShader.setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
        plantShader.setVec3("dirLight.specular", glm::vec3(0.1f));

        plantShader.setVec3("pointLight.position", lightPos);
        plantShader.setVec3("pointLight.ambient", glm::vec3(0.1f));
        plantShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        plantShader.setVec3("pointLight.specular", 0.0f, 0.0f, 0.0f);
        plantShader.setFloat("pointLight.constant", 1.0f);
        plantShader.setFloat("pointLight.linear", 0.007f);
        plantShader.setFloat("pointLight.quadratic", 0.0002f);

        // TODO: Da li ostaviti ovo (camera.*)?
        plantShader.setVec3("spotLight.position", camera.Position);
        plantShader.setVec3("spotLight.direction", camera.Front);
        plantShader.setVec3("spotLight.ambient", glm::vec3(0.1f));
        plantShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        plantShader.setVec3("spotLight.specular", glm::vec3(1.2f));
        plantShader.setFloat("spotLight.constant", 1.0f);
        plantShader.setFloat("spotLight.linear", 0.007f);
        plantShader.setFloat("spotLight.quadratic", 0.0002f);
        plantShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        plantShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, -4.0f));
        model = glm::scale(model, glm::vec3(0.3f));
        plantShader.setMat4("model", model);
        plant.Draw(plantShader);

        // Book with parallax mapping

        // TODO: prebaci na parallax occlusion metod
        for (int i = 0; i < (int)book.meshes[0].textures.size(); i++) {
            bookShader.setInt("material.texture_height1", book.meshes[0].textures.size());
            glActiveTexture(GL_TEXTURE0 + book.meshes[0].textures.size());
            glBindTexture(GL_TEXTURE_2D, heightMap);
            bookShader.setFloat("heightScale", heightScale);
        }

        bookShader.use();
        bookShader.setInt("flashLight", flashLight);
        bookShader.setMat4("projection", projection);
        bookShader.setMat4("view", view);
        bookShader.setVec3("lightPos", lightPos);
        bookShader.setVec3("lightDir", dirPos);
        bookShader.setVec3("viewPos", lightPos);
        bookShader.setFloat("material.shininess", 32.0f);
        // Salju se vertex shader-u
        bookShader.setVec3("spotPosition", camera.Position);
        bookShader.setVec3("spotDirection", camera.Front);

        bookShader.setVec3("dirLight.direction", glm::vec3(dirPos));
        bookShader.setVec3("dirLight.ambient", glm::vec3(0.1));
        bookShader.setVec3("dirLight.diffuse", 0.2f, 0.2f, 0.2f);
        bookShader.setVec3("dirLight.specular", glm::vec3(0.1f));

        bookShader.setVec3("pointLight.position", lightPos);
        bookShader.setVec3("pointLight.ambient", glm::vec3(0.1f));
        bookShader.setVec3("pointLight.diffuse", 1.0f, 1.0f, 1.0f);
        bookShader.setVec3("pointLight.specular", glm::vec3(0.1));
        bookShader.setFloat("pointLight.constant", 1.0f);
        bookShader.setFloat("pointLight.linear", 0.007f);
        bookShader.setFloat("pointLight.quadratic", 0.0002f);

        // TODO: Da li ostaviti ovo (camera.*)?
        bookShader.setVec3("spotLight.position", camera.Position);
        bookShader.setVec3("spotLight.direction", camera.Front);
        bookShader.setVec3("spotLight.ambient", glm::vec3(0.1f));
        bookShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
        bookShader.setVec3("spotLight.specular", glm::vec3(1.2f));
        bookShader.setFloat("spotLight.constant", 1.0f);
        bookShader.setFloat("spotLight.linear", 0.007f);
        bookShader.setFloat("spotLight.quadratic", 0.0002f);
        bookShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        bookShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        std::vector<std::pair<glm::vec3, float>> positions{
                make_pair(glm::vec3(-3.0f, 0.5f, -8.0f), glm::radians(90.0f)),
                make_pair(glm::vec3(-2.8f, 1.0f, -8.0f), glm::radians(90.0f)),
                make_pair(glm::vec3(-3.0f, 1.5f, -4.5f), glm::radians(-90.0f)),
                make_pair(glm::vec3(-1.0f, 0.2f, -8.0f), glm::radians(+90.0f))
        };

        int n = positions.size();
        for (int i = 0; i < n; i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, positions[i].first);
            if (i == n-1) {
                model = glm::rotate(model, positions[i].second, glm::vec3(0.0, 1.0, 0.0));
                model = glm::rotate(model, glm::radians(-23.0f), glm::vec3(1.0, 0.0, 0.0));
            }
            else
                model = glm::rotate(model, positions[i].second, glm::vec3(1.0, 0.0, 0.0));
            model = glm::scale(model, glm::vec3(0.8f));
            bookShader.setMat4("model", model);
            book.Draw(bookShader);
        }

        // Lighting cube defining

        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(1.5f));
        lightCubeShader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
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

        if(AABloom){
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // blur bright fragments with two-pass Gaussian Blur
            // --------------------------------------------------
            bool horizontal = true, first_iteration = true;
            unsigned int amount = 10;
            shaderBlur.use();
            for (unsigned int i = 0; i < amount; i++)
            {
                glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
                shaderBlur.setInt("horizontal", horizontal);
                glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
                renderQuad();
                horizontal = !horizontal;
                if (first_iteration)
                    first_iteration = false;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
            // --------------------------------------------------------------------------------------------------------------------------
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            shaderBloomFinal.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
            shaderBloomFinal.setInt("bloom", bloom);
            shaderBloomFinal.setFloat("exposure", exposure);
            renderQuad();
        }else{
            // 2. now blit multisampled buffer(s) to normal colorbuffer of intermediate FBO. Image is stored in screenTexture
            glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFBO);
            glBlitFramebuffer(0, 0, SCR_WIDTH, SCR_HEIGHT, 0, 0, SCR_WIDTH, SCR_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

            // 3. now render quad with scene's visuals as its texture image
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);

            // draw Screen quad
            screenShader.use();
            glBindVertexArray(quadVAO);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, screenTexture); // use the now resolved color attachment as the quad's texture
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
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
        camera.MovementSpeed += 1.0f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.MovementSpeed -= 1.0f;

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
        glEnable(GL_MULTISAMPLE);
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS)
        glDisable(GL_MULTISAMPLE);

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !flashLightKeyPressed)
    {
        flashLight = !flashLight;
        flashLightKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
    {
        flashLightKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.1f;
        else
            exposure = 2.5f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        exposure += 0.1f;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !AABloomKeyPressed)
    {
        AABloom = !AABloom;
        AABloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        AABloomKeyPressed = false;
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
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to to

//    float sensitivity = 0.01f; // change this value to your liking
//    xoffset *= sensitivity;
//    yoffset *= sensitivity;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {

        // ako je ukljucen kursor - iskljuci ga
        if (cursorToggle) {
            cursorToggle = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            cursorToggle = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }


    }
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

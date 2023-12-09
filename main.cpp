#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <iostream>

#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_m.h"
#include "Camera.h"
#include "Light.h"
#include "Model.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

unsigned int loadTexture(const char* path);

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

Camera camera = Camera();
Light light = Light();

//mouse pos
float lastMouseX = SCR_WIDTH/2, lastMouseY = SCR_HEIGHT/2;

bool scrollInputHappened = false;

bool inputLeft = false;
bool inputRight = false;
bool inputUp = false;
bool inputDown = false;

bool inputForward = false;
bool inputBack = false;
bool inputAccelerate = false;
bool inputDecelerate = false;

bool inputEnabled = true;


int main()
{

    //Initialize the GLFW and set variables
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//version 4
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);//         .6
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    //making a window
    
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    //Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    //set the size of the viewport
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    //set the callback function for re-sizing the window
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //set scroll callback
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    //---------- Creating Window Complete ----------//


    //---------- Begin Creating Shaders and Programs ----------//
    Shader lightingShader = Shader("Shaders/Vertex.glsl", "Shaders/Fragment.glsl");
    Shader lightCubeShader = Shader("Shaders/Vertex_source.glsl", "Shaders/Fragment_source.glsl");
    Shader OutlineShader = Shader("Shaders/Vertex_Outline.glsl", "Shaders/SingleColorFragment.glsl");

    //---------- Finished Creating Shaders ----------//

    //---------- Load Objects  ----------//

    stbi_set_flip_vertically_on_load(true);


    //load backpack model
    //Model backpack = Model("assets/backpack/backpack.obj");

    vector<Vertex> cubeVertices;

    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));//back
    cubeVertices.push_back(Vertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex( 0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex( 0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
    cubeVertices.push_back(Vertex(-0.5f,  0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));

    cubeVertices.push_back(Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));//front
    cubeVertices.push_back(Vertex( 0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex( 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex( 0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f,  0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f));

    cubeVertices.push_back(Vertex(-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));//left
    cubeVertices.push_back(Vertex(-0.5f,  0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f));
    cubeVertices.push_back(Vertex(-0.5f,  0.5f,  0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f));

    cubeVertices.push_back(Vertex(0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));//right
    cubeVertices.push_back(Vertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(0.5f,  0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex(0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(0.5f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex(0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f));

    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f));//bottom
    cubeVertices.push_back(Vertex( 0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex( 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex( 0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f,  0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f));
    cubeVertices.push_back(Vertex(-0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f));

    cubeVertices.push_back(Vertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));//Top
    cubeVertices.push_back(Vertex( 0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex( 0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f));
    cubeVertices.push_back(Vertex( 0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f));
    cubeVertices.push_back(Vertex(-0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f));
    cubeVertices.push_back(Vertex(-0.5f, 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f));
    vector<unsigned int> cubeIndices = {
        0,1,2,3,4,5,
        6,7,8,9,10,11,
        12,13,14,15,16,17,
        18,19,20,21,22,23,
        24,25,26,27,28,29,
        30,31,32,33,34,35,
    };
    vector<Vertex> planeVertices;
    planeVertices.push_back(Vertex(-0.5f, -0.5f, 0, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f));
    planeVertices.push_back(Vertex( 0.5f, -0.5f, 0, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
    planeVertices.push_back(Vertex(-0.5f,  0.5f, 0, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));

    planeVertices.push_back(Vertex(-0.5f,  0.5f, 0, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f));
    planeVertices.push_back(Vertex( 0.5f, -0.5f, 0, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f));
    planeVertices.push_back(Vertex( 0.5f,  0.5f, 0, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f));

    vector<unsigned int> planeIndices = {
        0,1,2,
        3,4,5
    };


    //grab textures
    
    unsigned int diffuseMap = loadTexture("assets/container/container2.png");
    unsigned int specularMap = loadTexture("assets/container/container2_specular.png");

    vector<Texture> cubeTextures;
    cubeTextures.push_back(Texture(diffuseMap, "texture_diffuse", ""));
    cubeTextures.push_back(Texture(specularMap, "texture_Specular", ""));
    Mesh Container = Mesh(cubeVertices, cubeIndices, cubeTextures);

    vector<Texture> LightTextures;
    Mesh lightCube = Mesh(cubeVertices, cubeIndices, LightTextures);

    unsigned int grassTexture = loadTexture("assets/grass.png");
    vector<Texture> grassTextures;
    grassTextures.push_back(Texture(grassTexture, "texture_diffuse", ""));
    Mesh grassPlane = Mesh(planeVertices, planeIndices, grassTextures);

    //---------- Using Our Shader ----------//

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    // -------------------------------------------------------------------------------------------
    lightingShader.use(); // don't forget to activate/use the lightingShader before setting uniforms!

    //----------    ----------//

    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };


    vector<glm::vec3> vegetation;
    vegetation.push_back(glm::vec3(-1.5f, 0.0f, -0.48f));
    vegetation.push_back(glm::vec3(1.5f, 0.0f, 0.51f));
    vegetation.push_back(glm::vec3(0.0f, 0.0f, 0.7f));
    vegetation.push_back(glm::vec3(-0.3f, 0.0f, -2.3f));
    vegetation.push_back(glm::vec3(0.5f, 0.0f, -0.6f));
    
    
    

    lightingShader.use();
    //lightingShader.setInt("material.texture_diffuse1", 0);
    //lightingShader.setInt("material.texture_specular1", 1);
    lightingShader.setFloat("material.shininess", 32);

    // configure global opengl state
    // -----------------------------
    //depth
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    //stencils
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //blending / transparency
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //culling
    glEnable(GL_CULL_FACE);

    //IMGUI
    ImGui::CreateContext();
    ImGuiIO io = ImGui::GetIO(); (void)io;//?
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    

    //----------  Creating Lights ----------//

    DirectionalLight dirLight = DirectionalLight(glm::vec3(0,0,1.0f), glm::vec3(1.0f), 0.1f, 0.9f, 1.0f);
    vector<PointLight> PointLights;
    //                           Position                       Color      Ambient Diffuse Specular
    PointLight pl1 = PointLight(glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(1.0f), 0.05f, 0.8f, 1.0f);
    
    PointLight pl2 = PointLight(glm::vec3(2.3f, -3.3f, -4.0f), glm::vec3(1.0f), 0.05f, 0.8f, 1.0f);
    PointLight pl3 = PointLight(glm::vec3(-4.0f, 2.0f, -12.0f), glm::vec3(1.0f), 0.05f, 0.8f, 1.0f);
    PointLight pl4 = PointLight(glm::vec3(0.0f, 0.0f, -3.0f), glm::vec3(1.0f), 0.05f, 0.8f, 1.0f);

    PointLights.push_back(pl1);
    PointLights.push_back(pl2);
    PointLights.push_back(pl3);
    PointLights.push_back(pl4);

    SpotLight spotLight = SpotLight(camera.Position, camera.Front,//position, direction
        glm::vec3(1.0f), 0.0f, 1.0f, 1.0f,//color, ambient diffuse specular strengths
        1, 0.09f, 0.032f,//falloff values
        glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(15.0f))); // cutoff values

    //----------  Finished Creating Lights ----------//


    //---------- Render Loop ----------//
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        if (!io.WantCaptureMouse) {
            inputEnabled = true;
            processInput(window);
        }
        

        // render
        // ------
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        //enable z-buffer depth testing
        glEnable(GL_DEPTH_TEST);
        //glDepthMask(GL_FALSE);
        glDepthFunc(GL_LESS);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0x00); // make sure we don't update the stencil buffer while drawing normal things



        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        //lightingShader.setVec3("light.position", light.Position);
        lightingShader.setVec3("ViewPos", camera.Position);

        // light properties
        // directional light
        dirLight.UpdateShader(lightingShader, "dirLight");
        // point light 1
        pl1.UpdateShader(lightingShader, "pointLights[0]");
        // point light 2
        pl2.UpdateShader(lightingShader, "pointLights[1]");
        // point light 3
        pl3.UpdateShader(lightingShader, "pointLights[2]");
        // point light 4
        pl4.UpdateShader(lightingShader, "pointLights[3]");
        // spotLight
        spotLight.Position = camera.Position;
        spotLight.Direction = camera.Front;
        spotLight.UpdateShader(lightingShader, "spotLight");
        

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        lightingShader.setMat4("projection", projection);
        lightingShader.setMat4("view", view);
        
        // world transformation
        glm::mat4 model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);

        OutlineShader.use();
        OutlineShader.setMat4("projection", projection);
        OutlineShader.setMat4("view", view);

        lightingShader.use();
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, 0.5f, 0.0f));
        model = glm::scale(model, glm::vec3(0.5f));

        lightingShader.setMat4("model", model);

        //backpack.Draw(lightingShader);

        //draw the lamp object(s)
        lightCubeShader.use();
        lightCubeShader.setMat4("projection", projection);
        lightCubeShader.setMat4("view", view);

        // we now draw as many light bulbs as we have point lights.
        for (unsigned int i = 0; i < PointLights.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, PointLights[i].Position);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            lightCubeShader.setMat4("model", model);
            lightCubeShader.setVec3("lightColor", PointLights[i].SpecularColor);
            lightCube.Draw(lightCubeShader);
        }

        
        // render containers with outlines
        //draw container with stencil on

        lightingShader.use();
        glStencilFunc(GL_ALWAYS, 1, 0xFF);//always pass stencil test
        glStencilMask(0xFF); // enable drawing to stencil buffer

        model = glm::mat4(1.0f);
        lightingShader.setMat4("model", model);
        Container.Draw(lightingShader);

        model = glm::translate(model, glm::vec3(2, 1, 4));
        lightingShader.setMat4("model", model);
        Container.Draw(lightingShader);

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilMask(0x00);
        glEnable(GL_DEPTH_TEST);

        OutlineShader.use();
        OutlineShader.setBool("useNormals", false);
        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(1.1f));
        OutlineShader.setMat4("model", model);
        Container.Draw(OutlineShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2, 1, 4));
        model = glm::scale(model, glm::vec3(1.1f));
        OutlineShader.setMat4("model", model);
        Container.Draw(OutlineShader);

        glStencilMask(0x00);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        glEnable(GL_DEPTH_TEST);

        //--------- DRAW TRANSPARENT OBJECTS ---------//
        glEnable(GL_BLEND);

        lightingShader.use();
        for (unsigned int i = 0; i < vegetation.size(); i++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            lightingShader.setMat4("model", model);
            grassPlane.Draw(lightingShader);
            //glDrawElements(GL_TRIANGLES, 4, GL_UNSIGNED_INT, planeIndices);
        }

        //---------- END OF RENDERING  ----------//

        //---------- IMGUI  ----------//

        ImGui::DockSpaceOverViewport();
        
        ImGui::Begin("I'm a window! WOW!");
        ImGui::Text("Directional Light");
        //float direction[] = { 0,0,0 };
        ImGui::SliderFloat3("Light Direction", glm::value_ptr(dirLight.Direction), -1, 1);
        ImGui::ColorEdit3("Light Ambient", glm::value_ptr(dirLight.AmbientColor));
        ImGui::ColorEdit3("Light Diffuse", glm::value_ptr(dirLight.DiffuseColor));
        ImGui::ColorEdit3("Light Specular", glm::value_ptr(dirLight.SpecularColor));
        ImGui::End();

        ImGui::ShowDemoWindow();

        
        //---------- IMGUI RENDER  ----------//

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    lightCube.DeleteGLObjects();
    Container.DeleteGLObjects();
    grassPlane.DeleteGLObjects();

    glfwTerminate();
    return 0;
}

//Taking input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = 2.5f * deltaTime; // adjust accordingly
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
    
    /*if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {inputForward = true;} else {inputForward = false;}
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {inputLeft = true;} else {inputLeft = false;}
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {inputBack = true;} else { inputBack = false;}
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {inputRight = true;} else { inputRight = false;}
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {inputUp = true;} else { inputUp = false;}
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {inputDown = true;} else { inputDown = false;}*/
}

//This is called when the window is resized
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

//callback for scroll wheels
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

bool firstMouse = true;
bool rotateCamera = false;
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos; // reversed since y-coordinates go from bottom to top

    lastMouseX = xpos;
    lastMouseY = ypos;

    if (rotateCamera)
    camera.ProcessMouseMovement(xoffset, yoffset);
    
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rotateCamera = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        firstMouse = true;
    }
    else {
        rotateCamera = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGBA;
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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
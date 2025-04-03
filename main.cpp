#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "Shader.h"
#include "Camera.h"
#include "Model.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tinyfiledialogs.h"

#define SCREEN_WIDTH 1600
#define SCREEN_HEIGHT 900

// Function prototypes
void processInput(GLFWwindow* window, float deltaTime);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void focusOnSelectedModel(Camera& camera, const Model* model);

// Global variables
Camera camera(glm::vec3(0.0f, 0.0f, 10.0f)); // Adjusted camera position
float lastX = SCREEN_WIDTH / 2.0f, lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f, lastFrame = 0.0f;
std::vector<Model*> models;
std::vector<std::string> modelFiles;
int selectedModelIndex = -1;

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game Engine", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
    ImGui::StyleColorsDark();

    Shader shader("vertex_shader.glsl", "fragment_shader.glsl");
    shader.use();
    shader.setVec3("lightDir", glm::vec3(-0.2f, -1.0f, -0.3f));
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        for (std::size_t i = 0; i < models.size(); i++) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(i * 2.5f, 0.0f, -5.0f)); // Move models closer to the camera
            model = glm::scale(model, glm::vec3(0.01f)); // Scale down the model further
            shader.setMat4("model", model);

            // Debug output for model transformation
            std::cout << "Drawing model: " << modelFiles[i] << std::endl;

            models[i]->Draw(shader);
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Asset Manager");

        // **Import Model Button**
        if (ImGui::Button("Import Model")) {
            const char* filters[] = { "*.obj", "*.fbx", "*.glb" };
            const char* file = tinyfd_openFileDialog("Select a 3D Model", "", 3, filters, "3D Model Files", 0);
            if (file) {
                Model* newModel = new Model(file);
                if (newModel->IsLoaded()) {
                    models.push_back(newModel);
                    modelFiles.push_back(std::string(file));
                    std::cout << "Imported: " << file << " | Total Models: " << models.size() << std::endl;
                }
                else {
                    std::cerr << "Failed to load model: " << file << std::endl;
                    delete newModel;
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Imported Models:");

        // **List Box for Models**
        if (!modelFiles.empty()) {
            std::vector<const char*> fileNames;
            for (const std::string& name : modelFiles) {
                fileNames.push_back(name.c_str());
            }

            ImGui::ListBox("##Model List", &selectedModelIndex, fileNames.data(), fileNames.size(), 5);
        }

        // **Remove Model Button**
        if (selectedModelIndex >= 0 && selectedModelIndex < static_cast<int>(models.size())) {
            if (ImGui::Button("Remove Selected Model")) {
                delete models[selectedModelIndex];
                models.erase(models.begin() + selectedModelIndex);
                modelFiles.erase(modelFiles.begin() + selectedModelIndex);
                selectedModelIndex = -1;  // Reset selection
            }
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    for (Model* model : models) {
        delete model;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// **Fixed processInput function**
void processInput(GLFWwindow* window, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_W, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_S, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_A, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(GLFW_KEY_D, deltaTime);

    // Handle Ctrl+F to focus on the selected model
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (selectedModelIndex >= 0 && selectedModelIndex < static_cast<int>(models.size())) {
            focusOnSelectedModel(camera, models[selectedModelIndex]);
        }
    }
}

// **Fixed mouse_callback function**
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = SCREEN_WIDTH / 2.0f;
    static float lastY = SCREEN_HEIGHT / 2.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera.processMouseMovement(xoffset, yoffset);
}

// **Fixed scroll_callback function**
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processMouseScroll(static_cast<float>(yoffset));
}

// **Function to focus on the selected model**
void focusOnSelectedModel(Camera& camera, const Model* model) {
    if (!model) return;

    // Calculate the bounding box of the model
    glm::vec3 minBounds = glm::vec3(FLT_MAX);
    glm::vec3 maxBounds = glm::vec3(-FLT_MAX);
    
    for (const Mesh& mesh : model->meshes) {
        for (const Vertex& vertex : mesh.vertices) {
            minBounds = glm::min(minBounds, vertex.Position);
            maxBounds = glm::max(maxBounds, vertex.Position);
        }
    }

    // Calculate the center of the bounding box
    glm::vec3 center = (minBounds + maxBounds) * 0.5f;

    // Move the camera to a position near the model
    camera.Position = center + glm::vec3(0.0f, 0.0f, 5.0f); // Adjust the distance as needed
    camera.Front = glm::normalize(center - camera.Position);
    camera.updateCameraVectors();
}
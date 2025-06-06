#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "LoadImageNode.h"
#include "BrightnessContrastNode.h"
#include "ColorChannelSplitNode.h"

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Node Editor - LoadImageNode Test", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    LoadImageNode node(1);
    BrightnessContrastNode bcNode(2);
    ColorChannelSplitNode channelNode(3);

    // Register nodes
    Node::clearNodes();
    Node::registerNode(&node);
    Node::registerNode(&bcNode);
    Node::registerNode(&channelNode);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("LoadImageNode UI Test");
        node.drawUI();
        ImGui::End();

        // Process nodes in correct order, checking both direct dirty flag
        // and if input nodes are dirty
        if (node.dirty) {
            std::cout << "Processing Load Image Node..." << std::endl;
            node.process();
        }

        if (bcNode.dirty || (bcNode.inputs[0] && bcNode.inputs[0]->dirty)) {
            std::cout << "Processing Brightness/Contrast Node..." << std::endl;
            bcNode.process();
        }

        if (channelNode.dirty || (channelNode.inputs[0] && channelNode.inputs[0]->dirty)) {
            std::cout << "Processing Channel Split Node..." << std::endl;
            channelNode.process();
        }

        ImGui::Begin("BrightnessContrast Node Test");
        bcNode.drawUI();
        ImGui::End();

        ImGui::Begin("Color Channel Split Node");
        channelNode.drawUI();
        ImGui::End();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
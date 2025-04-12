#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


#include "LoadImageNode.h"
#include "BrightnessContrastNode.h"
#include "ColorChannelSplitNode.h"

int main() {
    // Init GLFW
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Node Editor - LoadImageNode Test", nullptr, nullptr);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Init GLEW
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Create LoadImageNode instance
    LoadImageNode node(1);
    BrightnessContrastNode bcNode(2);
    bcNode.setInput(0, &node);
    ColorChannelSplitNode channelNode(3);
    channelNode.setInput(0, &bcNode);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("LoadImageNode UI Test");
        node.drawUI(); // Draw the image loader node
        ImGui::End();

        if (node.dirty) node.process();
        if (bcNode.dirty) bcNode.process();
        if(channelNode.dirty) channelNode.process();

        ImGui::Begin("BrightnessContrast Node Test");
        bcNode.drawUI();
        ImGui::End();

        ImGui::Begin("Color Channel Split Node");
        channelNode.drawUI();
        ImGui::End();

        

        ImGui::Render();
        int display_w, display_h;
        glfwMakeContextCurrent(window);
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

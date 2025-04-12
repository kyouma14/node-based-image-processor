#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "LoadImageNode.h"
#include "BrightnessContrastNode.h"
#include "ColorChannelSplitNode.h"
#include "blurnode.h"
#include "ThresholdNode.h"
#include "EdgeDetectionNode.h"

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
    BlurNode blurNode(4);
    ThresholdNode threshNode(5);
    EdgeDetectionNode edgeNode(6);

    // Register nodes
    Node::clearNodes();
    Node::registerNode(&node);
    Node::registerNode(&bcNode);
    Node::registerNode(&blurNode);
    Node::registerNode(&channelNode);
    Node::registerNode(&threshNode);
    Node::registerNode(&edgeNode);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("LoadImageNode UI Test");
        node.drawUI();
        ImGui::End();
        

        if (node.dirty) {
            node.process();
        }

        if (bcNode.dirty || (bcNode.inputs[0] && bcNode.inputs[0]->dirty)) {
            bcNode.process();
        }

        if (channelNode.dirty || (channelNode.inputs[0] && channelNode.inputs[0]->dirty)) {
            channelNode.process();
        }


        if (blurNode.dirty || (blurNode.inputs[0] && blurNode.inputs[0]->dirty)) {
            blurNode.process();
        }

        if (threshNode.dirty || (threshNode.inputs[0] && threshNode.inputs[0]->dirty)) {
            threshNode.process();
        }

        if(edgeNode.dirty || (edgeNode.inputs[0] && edgeNode.inputs[0]->dirty)) {
            edgeNode.process();
        }

        ImGui::Begin("BrightnessContrast Node Test");
        bcNode.drawUI();
        ImGui::End();

        ImGui::Begin("Color Channel Split Node");
        channelNode.drawUI();
        ImGui::End();

        ImGui::Begin("Blur Node");
        blurNode.drawUI();
        ImGui::End();

        ImGui::Begin("Threshold Node");
        threshNode.drawUI();
        ImGui::End();

        ImGui::Begin("Edge Detection Node");
        edgeNode.drawUI();
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
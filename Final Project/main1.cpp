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
#include "BlendNode.h"
#include "NoiseGenerationNode.h"
#include "ConvolutionFilterNode.h"
#include "OutputNode.h"




int main() {
    
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(1500, 720, "Node Editor - LoadImageNode Test", nullptr, nullptr);
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
    BlendNode blendNode(7);
    NoiseGenerationNode noiseNode(8);
    ConvolutionFilterNode convNode(9);
    OutputNode outputNode(10);

    // Register nodes
    Node::clearNodes();
    Node::registerNode(&node);
    Node::registerNode(&bcNode);
    Node::registerNode(&blurNode);
    Node::registerNode(&channelNode);
    Node::registerNode(&threshNode);
    Node::registerNode(&edgeNode);
    Node::registerNode(&blendNode);
    Node::registerNode(&noiseNode);
    Node::registerNode(&convNode);
    Node::registerNode(&outputNode);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

        ImVec2 screen_size = ImGui::GetIO().DisplaySize;

        ImU32 top_color = IM_COL32(40, 40, 80, 255);
        ImU32 bottom_color = IM_COL32(10, 10, 30, 255);

        draw_list->AddRectFilledMultiColor(
            ImVec2(0, 0),
            ImVec2(screen_size.x, screen_size.y),
            top_color, top_color, bottom_color, bottom_color
        );
        
        ImGui::SetNextWindowSize(ImVec2(350,380), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(10,10), ImGuiCond_Once);
        ImGui::Begin("Input Node");
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

        if (blendNode.dirty || (blendNode.inputs[0] && blendNode.inputs[0]->dirty)) {
            blendNode.process();
        }

        if (noiseNode.dirty) {
            noiseNode.process();
        }

        if (convNode.dirty || (convNode.inputs[0] && convNode.inputs[0]->dirty)) {
            convNode.process();
        }

        if (outputNode.dirty || (outputNode.inputs[0] && outputNode.inputs[0]->dirty)) {
            outputNode.process();
        }
        
        ImGui::SetNextWindowSize(ImVec2(350,640), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(10,400), ImGuiCond_Once);
        ImGui::Begin("BrightnessContrast Node Test");
        bcNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,380), ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(370,10), ImGuiCond_Once);
        ImGui::Begin("Color Channel Split Node");
        channelNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,640),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(370,400), ImGuiCond_Once);
        ImGui::Begin("Blur Node");
        blurNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,640),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(730,400), ImGuiCond_Once);
        ImGui::Begin("Threshold Node");
        threshNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,380),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(730,10), ImGuiCond_Once);
        ImGui::Begin("Edge Detection Node");
        edgeNode.drawUI();
        ImGui::End();
        
        ImGui::SetNextWindowSize(ImVec2(350,640),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(1090,400), ImGuiCond_Once);
        ImGui::Begin("Blend Node");
        blendNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,640),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(1450,400), ImGuiCond_Once);
        ImGui::Begin("Noise Generation Node");
        noiseNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,380),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(1090,10), ImGuiCond_Once);
        ImGui::Begin("Convolution Filter Node");
        convNode.drawUI();
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(350,380),ImGuiCond_Once);
        ImGui::SetNextWindowPos(ImVec2(1450,10), ImGuiCond_Once);
        ImGui::Begin("Output Node");
        outputNode.drawUI();
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
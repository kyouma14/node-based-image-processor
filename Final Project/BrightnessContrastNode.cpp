#include "BrightnessContrastNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include <GL/glew.h>

BrightnessContrastNode::BrightnessContrastNode(int id)
    : Node(id, "Brightness/Contrast") {
    inputs.resize(1);
}

void BrightnessContrastNode::process() {
    std::cout << "Processing BrightnessContrastNode..." << std::endl;
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            std::cout << "Input image size: " << input.size() << " channels: " << input.channels() << std::endl;
            input.convertTo(output, -1, contrast, brightness);
            std::cout << "Output image size: " << output.size() << " channels: " << output.channels() << std::endl;

            if (texture) {
                glDeleteTextures(1, &texture);
                texture = 0;
            }
            texture = matToTexture(output);
        } else {
            std::cout << "Input image is empty!" << std::endl;
        }
    } else {
        std::cout << "No input connected!" << std::endl;
        output = cv::Mat();
        if (texture) {
            glDeleteTextures(1, &texture);
            texture = 0;
        }
    }
    dirty = false;
}

cv::Mat BrightnessContrastNode::getOutput() const {
    return output;
}

void BrightnessContrastNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();  // top left of content region
    ImVec2 size = ImGui::GetContentRegionAvail();  // drawable area
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);  // bottom right

    // Gradient colors
    ImVec4 color1 = ImVec4(70.0f / 255.0f, 70.0f / 255.0f, 70.0f / 255.0f, 1.0f); 
    ImVec4 color2 = ImVec4(40.0f / 255.0f, 40.0f / 255.0f, 40.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),  // top left to top right
        ImColor(color2), ImColor(color2)); // bottom left to bottom right

    // UI on top
    ImGui::SetCursorScreenPos(p0);
    // Input selection dropdown
    ImGui::Text("Input Selection:");
    if (ImGui::BeginCombo("Input Image##BC", 
        inputs[0] ? inputs[0]->getName().c_str() : "None")) {
        
        if (ImGui::Selectable("None", inputs[0] == nullptr)) {
            setInput(0, nullptr);
            markDirty();
        }
        
        for (Node* node : Node::availableNodes) {
            if (node != this) {
                bool is_selected = (inputs[0] == node);
                if (ImGui::Selectable(node->getName().c_str(), is_selected)) {
                    setInput(0, node);
                    markDirty();
                }
            }
        }
        ImGui::EndCombo();
    }

    // Brightness and contrast controls
    ImGui::Text("Adjust Brightness & Contrast");

    if (ImGui::SliderFloat("Brightness", &brightness, -100.0f, 100.0f)) {
        markDirty();
    }

    if (ImGui::Button("Reset Brightness")) {
        brightness = 0.0f;
        markDirty();
    }

    ImGui::Spacing();

    if (ImGui::SliderFloat("Contrast", &contrast, 0.0f, 3.0f)) {
        markDirty();
    }

    if (ImGui::Button("Reset Contrast")) {
        contrast = 1.0f;
        markDirty();
    }

    if (texture) {
        ImGui::Spacing();
        ImGui::Text("Preview:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

GLuint BrightnessContrastNode::matToTexture(const cv::Mat& mat) {
    cv::Mat rgbMat;
    GLenum format;

    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        format = GL_RGB;
    } else {
        rgbMat = mat;
        format = GL_LUMINANCE;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbMat.cols, rgbMat.rows, 0, format, GL_UNSIGNED_BYTE, rgbMat.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}
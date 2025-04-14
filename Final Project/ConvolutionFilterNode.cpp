#include "ConvolutionFilterNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

ConvolutionFilterNode::ConvolutionFilterNode(int id) : Node(id, "Convolution Filter") {
    inputs.resize(1);
    resetKernel();
}

void ConvolutionFilterNode::resetKernel() {
    int size = kernelSize * kernelSize;
    kernel.resize(size, 0.0f);
    kernel[size / 2] = 1.0f;  // Center pixel
    kernelDivisor = 1.0f;
    kernelOffset = 0.0f;
}

void ConvolutionFilterNode::process() {
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            output = applyKernel(input);

            if (texture) {
                glDeleteTextures(1, &texture);
            }
            texture = matToTexture(output);
            
            // Update preview
            updatePreview();
        }
    }
    dirty = false;
}

cv::Mat ConvolutionFilterNode::applyKernel(const cv::Mat& input) {
    cv::Mat result;
    cv::Mat kernelMat(kernelSize, kernelSize, CV_32F);
    
    // Convert our kernel vector to Mat
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            kernelMat.at<float>(i, j) = kernel[i * kernelSize + j] / kernelDivisor;
        }
    }

    // Apply convolution
    cv::filter2D(input, result, -1, kernelMat, cv::Point(-1, -1), kernelOffset);
    return result;
}

void ConvolutionFilterNode::applyPreset(Preset preset) {
    switch (preset) {
        case Preset::Sharpen:
            kernelSize = 3;
            kernel = {0, -1, 0, -1, 5, -1, 0, -1, 0};
            kernelDivisor = 1.0f;
            break;

        case Preset::Emboss:
            kernelSize = 3;
            kernel = {-2, -1, 0, -1, 1, 1, 0, 1, 2};
            kernelDivisor = 1.0f;
            kernelOffset = 128.0f;
            break;

        case Preset::EdgeEnhance:
            kernelSize = 3;
            kernel = {0, 0, 0, -1, 1, 0, 0, 0, 0};
            kernelDivisor = 1.0f;
            break;

        case Preset::BoxBlur:
            kernelSize = 3;
            kernel = {1, 1, 1, 1, 1, 1, 1, 1, 1};
            kernelDivisor = 9.0f;
            break;

        case Preset::GaussianBlur:
            kernelSize = 5;
            kernel = {
                1, 4, 6, 4, 1,
                4, 16, 24, 16, 4,
                6, 24, 36, 24, 6,
                4, 16, 24, 16, 4,
                1, 4, 6, 4, 1
            };
            kernelDivisor = 256.0f;
            break;

        case Preset::EdgeDetect:
            kernelSize = 3;
            kernel = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
            kernelDivisor = 1.0f;
            break;

        case Preset::Custom:
            resetKernel();
            break;
    }
    markDirty();
}

void ConvolutionFilterNode::updatePreview() {
    // Create a simple gradient image for preview
    cv::Mat previewInput(100, 100, CV_8UC1);
    for (int i = 0; i < previewInput.rows; i++) {
        for (int j = 0; j < previewInput.cols; j++) {
            previewInput.at<uchar>(i, j) = (i + j) * 255 / (previewInput.rows + previewInput.cols);
        }
    }

    // Apply kernel to preview
    cv::Mat previewResult = applyKernel(previewInput);

    // Update preview texture
    if (previewTexture) {
        glDeleteTextures(1, &previewTexture);
    }
    previewTexture = matToTexture(previewResult);
}

void ConvolutionFilterNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();  // top left of content region
    ImVec2 size = ImGui::GetContentRegionAvail();  // drawable area
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);  // bottom right

    // Gradient colors
    ImVec4 color1 = ImVec4(85.0f / 255.0f, 107.0f / 255.0f, 47.0f / 255.0f, 1.0f);
    ImVec4 color2 = ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),  // top left to top right
        ImColor(color2), ImColor(color2)); // bottom left to bottom right

    // UI on top
    ImGui::SetCursorScreenPos(p0); 
    // Input selection
    ImGui::Text("Input Image:");
    if (ImGui::BeginCombo("Input##Conv", 
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

    // Preset selection
    const char* presets[] = {"Custom", "Sharpen", "Emboss", "Edge Enhance", 
                            "Box Blur", "Gaussian Blur", "Edge Detect"};
    int presetIndex = static_cast<int>(currentPreset);
    if (ImGui::Combo("Preset", &presetIndex, presets, IM_ARRAYSIZE(presets))) {
        currentPreset = static_cast<Preset>(presetIndex);
        applyPreset(currentPreset);
    }

    // Kernel size selection (only for custom)
    if (currentPreset == Preset::Custom) {
        if (ImGui::RadioButton("3x3", kernelSize == 3)) {
            kernelSize = 3;
            resetKernel();
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("5x5", kernelSize == 5)) {
            kernelSize = 5;
            resetKernel();
            markDirty();
        }
    }

    // Kernel matrix editor
    ImGui::Text("Kernel Matrix:");
    bool kernelChanged = false;
    for (int i = 0; i < kernelSize; i++) {
        for (int j = 0; j < kernelSize; j++) {
            if (j > 0) ImGui::SameLine();
            float& value = kernel[i * kernelSize + j];
            std::string label = "##K" + std::to_string(i) + std::to_string(j);
            if (ImGui::DragFloat(label.c_str(), &value, 0.1f, -10.0f, 10.0f, "%.3f")) {
                kernelChanged = true;
            }
        }
    }

    // Kernel parameters
    if (ImGui::DragFloat("Divisor", &kernelDivisor, 0.1f, 0.1f, 1000.0f, "%.3f")) {
        kernelChanged = true;
    }
    if (ImGui::DragFloat("Offset", &kernelOffset, 1.0f, -255.0f, 255.0f)) {
        kernelChanged = true;
    }

    if (kernelChanged) {
        markDirty();
    }

    // Preview
    if (previewTexture) {
        ImGui::Text("Kernel Effect Preview:");
        ImGui::Image((ImTextureID)(intptr_t)previewTexture, ImVec2(100, 100));
    }

    // Result
    if (texture) {
        ImGui::Text("Result:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat ConvolutionFilterNode::getOutput() const {
    return output;
}

GLuint ConvolutionFilterNode::matToTexture(const cv::Mat& mat) {
    cv::Mat rgbMat;
    GLenum format;

    if (mat.channels() == 3) {
        cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB);
        format = GL_RGB;
    } else {
        cv::cvtColor(mat, rgbMat, cv::COLOR_GRAY2RGB);
        format = GL_RGB;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, rgbMat.cols, rgbMat.rows, 0, format, GL_UNSIGNED_BYTE, rgbMat.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureID;
}
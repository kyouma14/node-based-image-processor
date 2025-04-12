#include "ColorChannelSplitNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include <GL/glew.h>

ColorChannelSplitNode::ColorChannelSplitNode(int id)
    : Node(id, "Channel Splitter") {
    inputs.resize(1);
    outputs.resize(4);
    grayscale = false;
    for (int i = 0; i < 3; ++i) {
        textures[i] = 0;
    }
}

void ColorChannelSplitNode::process() {
    std::cout << "Processing ColorChannelSplitNode..." << std::endl;
    
    // Clear existing textures
    for (int i = 0; i < 3; ++i) {
        if (textures[i]) {
            glDeleteTextures(1, &textures[i]);
            textures[i] = 0;
        }
    }

    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty() && input.channels() >= 3) {
            std::cout << "Input image size: " << input.size() << " channels: " << input.channels() << std::endl;
            
            std::vector<cv::Mat> channels;
            cv::split(input, channels);

            for (int i = 0; i < 3; ++i) {
                if (grayscale) {
                    // Store direct grayscale channel
                    if (i == 0) redChannel = channels[i];
                    else if (i == 1) greenChannel = channels[i];
                    else if (i == 2) blueChannel = channels[i];
                } else {
                    // Create colored version of each channel
                    std::vector<cv::Mat> merged(3, cv::Mat::zeros(channels[i].size(), channels[i].type()));
                    merged[i] = channels[i];
                    cv::Mat mergedChannel;
                    cv::merge(merged, mergedChannel);

                    if (i == 0) redChannel = mergedChannel;
                    else if (i == 1) greenChannel = mergedChannel;
                    else if (i == 2) blueChannel = mergedChannel;
                }

                // Create texture for each channel
                cv::Mat& channelMat = (i == 0) ? redChannel : (i == 1) ? greenChannel : blueChannel;
                if (!channelMat.empty()) {
                    textures[i] = matToTexture(channelMat);
                }
            }
        } else {
            std::cout << "Input image is empty or has insufficient channels!" << std::endl;
        }
    } else {
        std::cout << "No input connected!" << std::endl;
        redChannel = cv::Mat();
        greenChannel = cv::Mat();
        blueChannel = cv::Mat();
    }
    
    dirty = false;
}

cv::Mat ColorChannelSplitNode::getOutput() const {
    switch (selectedChannel) {
        case 0: return redChannel;
        case 1: return greenChannel;
        case 2: return blueChannel;
        default: return redChannel;
    }
}

void ColorChannelSplitNode::drawUI() {
    // Add input selection dropdown
    ImGui::Text("Input Selection:");
    if (ImGui::BeginCombo("Input Image##CS", 
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

    ImGui::Text("Color Channel Splitter");

    if (ImGui::Checkbox("Grayscale Output", &grayscale)) {
        markDirty();
    }

    //channel selectiom
    const char* channels[] = { "Red Channel", "Green Channel", "Blue Channel" };
    if (ImGui::Combo("Output Channel", &selectedChannel, channels, IM_ARRAYSIZE(channels))) {
        markDirty();
    }

    const char* labels[3] = { "Red", "Green", "Blue" };
    for (int i = 0; i < 3; ++i) {
        if (textures[i]) {
            if (i == selectedChannel){
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
            }
            ImGui::Text("%s Channel:", labels[i]);
            ImGui::Image((ImTextureID)(intptr_t)textures[i], ImVec2(150, 150));

            if (i == selectedChannel) {
                ImGui::PopStyleColor();
            }
        }
    }
}

GLuint ColorChannelSplitNode::matToTexture(const cv::Mat& mat) {
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

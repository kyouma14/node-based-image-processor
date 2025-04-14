#include "BlendNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include "tinyfiledialogs.h"

BlendNode::BlendNode(int id) : Node(id, "Blend") {
    inputs.resize(1);  // One input from node system
    secondImageTexture = 0;
    texture = 0;
    opacity = 1.0f;
    blendMode = 0;
}

void BlendNode::process() {
    if (inputs[0] && !secondImage.empty()) {
        cv::Mat baseImage = inputs[0]->getOutput();
        
        if (!baseImage.empty()) {
            cv::Mat resizedSecondImage;
            if (baseImage.size() != secondImage.size()) {
                cv::resize(secondImage, resizedSecondImage, baseImage.size());
            } else {
                resizedSecondImage = secondImage;
            }

            output = applyBlend(baseImage, resizedSecondImage, blendMode, opacity);

            if (texture) {
                glDeleteTextures(1, &texture);
            }
            texture = matToTexture(output);
        }
    }
    dirty = false;
}

cv::Mat BlendNode::applyBlend(const cv::Mat& base, const cv::Mat& blend, int mode, float opacity) {
    cv::Mat result;
    switch (mode) {
        case 1: // Multiply
            result = multiplyBlend(base, blend);
            break;
        case 2: // Screen
            result = screenBlend(base, blend);
            break;
        case 3: // Overlay
            result = overlayBlend(base, blend);
            break;
        case 4: // Difference
            result = differenceBlend(base, blend);
            break;
        default: // Normal
            cv::addWeighted(base, 1.0, blend, opacity, 0.0, result);
            break;
    }
    return result;
}

cv::Mat BlendNode::multiplyBlend(const cv::Mat& base, const cv::Mat& blend) {
    cv::Mat result = base.clone();
    for (int i = 0; i < base.rows; i++) {
        for (int j = 0; j < base.cols; j++) {
            for (int c = 0; c < 3; c++) {
                float baseVal = base.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float blendVal = blend.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float finalVal = baseVal * blendVal;
                result.at<cv::Vec3b>(i, j)[c] = cv::saturate_cast<uchar>(finalVal * 255.0f);
            }
        }
    }
    return result;
}

cv::Mat BlendNode::screenBlend(const cv::Mat& base, const cv::Mat& blend) {
    cv::Mat result = base.clone();
    for (int i = 0; i < base.rows; i++) {
        for (int j = 0; j < base.cols; j++) {
            for (int c = 0; c < 3; c++) {
                float baseVal = base.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float blendVal = blend.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float finalVal = 1.0f - (1.0f - baseVal) * (1.0f - blendVal);
                result.at<cv::Vec3b>(i, j)[c] = cv::saturate_cast<uchar>(finalVal * 255.0f);
            }
        }
    }
    return result;
}

cv::Mat BlendNode::overlayBlend(const cv::Mat& base, const cv::Mat& blend) {
    cv::Mat result = base.clone();
    for (int i = 0; i < base.rows; i++) {
        for (int j = 0; j < base.cols; j++) {
            for (int c = 0; c < 3; c++) {
                float baseVal = base.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float blendVal = blend.at<cv::Vec3b>(i, j)[c] / 255.0f;
                float finalVal;
                if (baseVal < 0.5f)
                    finalVal = 2.0f * baseVal * blendVal;
                else
                    finalVal = 1.0f - 2.0f * (1.0f - baseVal) * (1.0f - blendVal);
                result.at<cv::Vec3b>(i, j)[c] = cv::saturate_cast<uchar>(finalVal * 255.0f);
            }
        }
    }
    return result;
}

cv::Mat BlendNode::differenceBlend(const cv::Mat& base, const cv::Mat& blend) {
    cv::Mat result = base.clone();
    for (int i = 0; i < base.rows; i++) {
        for (int j = 0; j < base.cols; j++) {
            for (int c = 0; c < 3; c++) {
                int diff = std::abs(base.at<cv::Vec3b>(i, j)[c] - blend.at<cv::Vec3b>(i, j)[c]);
                result.at<cv::Vec3b>(i, j)[c] = cv::saturate_cast<uchar>(diff);
            }
        }
    }
    return result;
}

void BlendNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();  // top left of content region
    ImVec2 size = ImGui::GetContentRegionAvail();  // drawable area
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);  // bottom-right

    // Gradient colors
    ImVec4 color1 = ImVec4(85.0f / 255.0f, 0.0f / 255.0f, 40.0f / 255.0f, 1.0f);  
    ImVec4 color2 = ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),  // top left to top right
        ImColor(color2), ImColor(color2)); // bottom left to bottom right

    // UI on top
    ImGui::SetCursorScreenPos(p0);
    // Input selection for node input
    ImGui::Text("Input Image:");
    if (ImGui::BeginCombo("Input##Blend", 
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

    // Second image loader
    if (ImGui::Button("Load Second Image")) {
        const char* filters[] = { "*.jpg", "*.png", "*.bmp" };
        const char* selected = tinyfd_openFileDialog(
            "Open Second Image",
            "",
            3,
            filters,
            "Image files",
            0
        );

        if (selected) {
            secondImagePath = selected;
            secondImage = cv::imread(secondImagePath);
            if (!secondImage.empty()) {
                if (secondImageTexture) {
                    glDeleteTextures(1, &secondImageTexture);
                }
                secondImageTexture = matToTexture(secondImage);
                markDirty();
            }
        }
    }

    // Preview second image if loaded
    if (secondImageTexture) {
        ImGui::Text("Second Image:");
        ImGui::Image((ImTextureID)(intptr_t)secondImageTexture, ImVec2(150, 150));
    }

    // Blend mode selection
    const char* modes[] = { "Normal", "Multiply", "Screen", "Overlay", "Difference" };
    if (ImGui::Combo("Blend Mode", &blendMode, modes, IM_ARRAYSIZE(modes))) {
        markDirty();
    }

    // Opacity slider
    if (ImGui::SliderFloat("Opacity", &opacity, 0.0f, 1.0f)) {
        markDirty();
    }

    // Display result
    if (texture) {
        ImGui::Text("Result:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat BlendNode::getOutput() const {
    return output;
}

GLuint BlendNode::matToTexture(const cv::Mat& mat) {
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
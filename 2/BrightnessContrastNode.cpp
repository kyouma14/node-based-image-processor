#include "BrightnessContrastNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include <GL/glew.h>

BrightnessContrastNode::BrightnessContrastNode(int id)
    : Node(id, "Brightness/Contrast") {
    inputs.resize(1);  // 1 input (image)
}

void BrightnessContrastNode::process() {
    std::cout << "Processing BrightnessContrastNode..." << std::endl;
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            input.convertTo(output, -1, contrast, brightness);

            if (texture) glDeleteTextures(1, &texture);
            texture = matToTexture(output);
        }
    }
    dirty = false;
}

cv::Mat BrightnessContrastNode::getOutput() const {
    return output;
}

void BrightnessContrastNode::drawUI() {
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

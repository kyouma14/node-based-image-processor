// LoadImageNode.cpp
#include "LoadImageNode.h"
#include "tinyfiledialogs.h"
#include <imgui.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

LoadImageNode::LoadImageNode(int id)
    : Node(id, "Load Image") {}

void LoadImageNode::process() {
    if (!filePath.empty()) {
        image = cv::imread(filePath);
        if (!image.empty()) {
            if (texture) glDeleteTextures(1, &texture);
            texture = matToTexture(image);
        }
    }
    dirty = false;
}

cv::Mat LoadImageNode::getOutput() const {
    return image;
}

void LoadImageNode::drawUI() {
    if (ImGui::Button("Choose Image")) {
        const char* filters[] = { "*.jpg", "*.png", "*.bmp" };
        const char* selected = tinyfd_openFileDialog(
            "Open Image",
            "",
            3,
            filters,
            "Image files",
            0
        );

        if (selected) {
            filePath = selected;
            markDirty(); // Mark for processing
            process();
        }
    }

    if (texture) {
        ImGui::Text("Preview:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
        ;

    }
}

GLuint LoadImageNode::matToTexture(const cv::Mat& mat) {
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

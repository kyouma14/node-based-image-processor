#include "ColorChannelSplitNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include <GL/glew.h>

ColorChannelSplitNode::ColorChannelSplitNode(int id)
    : Node(id, "Channel Splitter") {
    inputs.resize(1); // One input image
    outputs.resize(4); // R, G, B, (A if applicable)
    grayscale = false;
    for (int i = 0; i < 3; ++i) {
        textures[i] = 0;
        

    }
}

void ColorChannelSplitNode::process() {
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty() && input.channels() >= 3) {
            std::vector<cv::Mat> channels;
            cv::split(input, channels);

            for (int i = 0; i < 3; ++i) {
                if (textures[i]) {
                    glDeleteTextures(1, &textures[i]);
                    textures[i] = 0;
                }

                if (grayscale) {
                    if (i == 0) redChannel = channels[i];
                    else if (i == 1) greenChannel = channels[i];
                    else if (i == 2) blueChannel = channels[i];

                } else {
                    // Merge single channel with zeros for RGB display
                    std::vector<cv::Mat> merged = {cv::Mat::zeros(channels[i].size(), channels[i].type()),
                                                   cv::Mat::zeros(channels[i].size(), channels[i].type()),
                                                   cv::Mat::zeros(channels[i].size(), channels[i].type())};
                    merged[i] = channels[i];
                    if (i == 0) redChannel = cv::Mat();
                    else if (i == 1) greenChannel = cv::Mat();
                    else if (i == 2) blueChannel = cv::Mat();

                    cv::Mat mergedChannel;
                    cv::merge(merged, mergedChannel);

                    if (i == 0) redChannel = mergedChannel;
                    else if (i == 1) greenChannel = mergedChannel;
                    else if (i == 2) blueChannel = mergedChannel;

                }

                if (i == 0) textures[i] = matToTexture(redChannel);
                else if (i == 1) textures[i] = matToTexture(greenChannel);
                else if (i == 2) textures[i] = matToTexture(blueChannel);

            }
        }
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
    ImGui::Text("Color Channel Splitter");

    if (ImGui::Checkbox("Grayscale Output", &grayscale)) {
        markDirty();
    }

    const char* labels[3] = { "Red", "Green", "Blue" };
    for (int i = 0; i < 3; ++i) {
        if (textures[i]) {
            ImGui::Text("%s Channel:", labels[i]);
            ImGui::Image((ImTextureID)(intptr_t)textures[i], ImVec2(150, 150));
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

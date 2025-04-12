#include "ThresholdNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

ThresholdNode::ThresholdNode(int id) : Node(id, "Threshold") {
    inputs.resize(1);
}

void ThresholdNode::process() {
    std::cout << "Processing Threshold Node..." << std::endl;
    
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            // Convert to grayscale if needed
            cv::Mat grayInput;
            if (input.channels() > 1) {
                cv::cvtColor(input, grayInput, cv::COLOR_BGR2GRAY);
            } else {
                grayInput = input;
            }

            // Update histogram
            updateHistogram(grayInput);

            // Apply thresholding
            if (useAdaptive) {
                cv::adaptiveThreshold(grayInput, output, maxValue,
                    adaptiveMethod,
                    cv::THRESH_BINARY,
                    blockSize,
                    C);
            } else {
                int flags = thresholdType;
                if (useOtsu) flags |= cv::THRESH_OTSU;
                cv::threshold(grayInput, output, thresholdValue, maxValue, flags);
            }

            // Update texture
            if (texture) {
                glDeleteTextures(1, &texture);
            }
            texture = matToTexture(output);
        }
    }
    dirty = false;
}

void ThresholdNode::drawUI() {
    // Input selection
    ImGui::Text("Input Selection:");
    if (ImGui::BeginCombo("Input Image##Threshold", 
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

    // Threshold controls
    ImGui::Text("Threshold Settings");

    // Method selection
    if (ImGui::Checkbox("Use Adaptive Threshold", &useAdaptive)) {
        markDirty();
    }

    if (!useAdaptive) {
        if (ImGui::Checkbox("Use Otsu's Method", &useOtsu)) {
            markDirty();
        }
    }

    if (!useAdaptive && !useOtsu) {
        if (ImGui::SliderInt("Threshold Value", &thresholdValue, 0, 255)) {
            markDirty();
        }
    }

    if (ImGui::SliderInt("Max Value", &maxValue, 0, 255)) {
        markDirty();
    }

    if (!useAdaptive) {
        const char* types[] = { "Binary", "Binary Inverted", "Truncate", "To Zero", "To Zero Inverted" };
        int currentType = thresholdType;
        if (ImGui::Combo("Threshold Type", &currentType, types, IM_ARRAYSIZE(types))) {
            thresholdType = currentType;
            markDirty();
        }
    } else {
        const char* methods[] = { "Mean", "Gaussian" };
        int currentMethod = (adaptiveMethod == cv::ADAPTIVE_THRESH_MEAN_C) ? 0 : 1;
        if (ImGui::Combo("Adaptive Method", &currentMethod, methods, IM_ARRAYSIZE(methods))) {
            adaptiveMethod = (currentMethod == 0) ? cv::ADAPTIVE_THRESH_MEAN_C : cv::ADAPTIVE_THRESH_GAUSSIAN_C;
            markDirty();
        }

        if (ImGui::SliderInt("Block Size", &blockSize, 3, 99, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            // Ensure block size is odd
            blockSize = (blockSize / 2) * 2 + 1;
            markDirty();
        }

        float cValue = (float)C;
        if (ImGui::SliderFloat("C", &cValue, -10.0f, 10.0f)) {
            C = (double)cValue;
            markDirty();
}
    }

    // Display histogram
    if (histogramTexture) {
        ImGui::Text("Histogram:");
        ImGui::Image((ImTextureID)(intptr_t)histogramTexture, ImVec2(256, 100));
    }

    // Display result
    if (texture) {
        ImGui::Text("Result:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

void ThresholdNode::updateHistogram(const cv::Mat& input) {
    if (input.empty()) return;

    // Calculate histogram
    std::vector<int> histogram(256, 0);
    for (int i = 0; i < input.rows; i++) {
        for (int j = 0; j < input.cols; j++) {
            histogram[input.at<uchar>(i, j)]++;
        }
    }

    // Find maximum value for scaling
    int maxCount = *std::max_element(histogram.begin(), histogram.end());

    // Create histogram image
    cv::Mat histImage(100, 256, CV_8UC3, cv::Scalar(0, 0, 0));
    for (int i = 0; i < 256; i++) {
        int height = static_cast<int>((histogram[i] * 100.0) / maxCount);
        cv::line(histImage,
            cv::Point(i, 100),
            cv::Point(i, 100 - height),
            cv::Scalar(255, 255, 255));
    }

    // Draw threshold line if not using adaptive
    if (!useAdaptive && !useOtsu) {
        cv::line(histImage,
            cv::Point(thresholdValue, 0),
            cv::Point(thresholdValue, 100),
            cv::Scalar(0, 0, 255),
            1);
    }

    // Update texture
    if (histogramTexture) {
        glDeleteTextures(1, &histogramTexture);
    }
    histogramTexture = matToTexture(histImage);
}

cv::Mat ThresholdNode::getOutput() const {
    return output;
}

GLuint ThresholdNode::matToTexture(const cv::Mat& mat) {
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
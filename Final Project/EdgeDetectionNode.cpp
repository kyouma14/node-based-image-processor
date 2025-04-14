#include "EdgeDetectionNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

EdgeDetectionNode::EdgeDetectionNode(int id) : Node(id, "Edge Detection") {
    inputs.resize(1);
}

void EdgeDetectionNode::process() {
    std::cout << "Processing Edge Detection Node..." << std::endl;
    
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

            // Apply edge detection
            cv::Mat edges;
            if (useCanny) {
                edges = applyCanny(grayInput);
            } else {
                edges = applySobel(grayInput);
            }

            // Create overlay if needed
            if (overlayEdges) {
                output = createOverlay(input, edges);
            } else {
                cv::cvtColor(edges, output, cv::COLOR_GRAY2BGR);
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

cv::Mat EdgeDetectionNode::applyCanny(const cv::Mat& input) {
    cv::Mat edges;
    cv::Canny(input, edges, cannyThreshold1, cannyThreshold2, cannyAperture);
    return edges;
}

cv::Mat EdgeDetectionNode::applySobel(const cv::Mat& input) {
    cv::Mat gradX, gradY, result;
    
    if (sobelX) {
        cv::Sobel(input, gradX, CV_16S, 1, 0, sobelKSize, sobelScale, sobelDelta);
    }
    
    if (sobelY) {
        cv::Sobel(input, gradY, CV_16S, 0, 1, sobelKSize, sobelScale, sobelDelta);
    }

    if (sobelX && sobelY) {
        cv::Mat absGradX, absGradY;
        cv::convertScaleAbs(gradX, absGradX);
        cv::convertScaleAbs(gradY, absGradY);
        cv::addWeighted(absGradX, 0.5, absGradY, 0.5, 0, result);
    } else if (sobelX) {
        cv::convertScaleAbs(gradX, result);
    } else {
        cv::convertScaleAbs(gradY, result);
    }

    return result;
}

cv::Mat EdgeDetectionNode::createOverlay(const cv::Mat& original, const cv::Mat& edges) {
    cv::Mat overlay = original.clone();
    for (int i = 0; i < overlay.rows; i++) {
        for (int j = 0; j < overlay.cols; j++) {
            if (edges.at<uchar>(i, j) > 0) {
                overlay.at<cv::Vec3b>(i, j) = cv::Vec3b(0, 0, 255); // Red edges
            }
        }
    }
    return overlay;
}

void EdgeDetectionNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();  // top left of content region
    ImVec2 size = ImGui::GetContentRegionAvail();  // drawable area
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);  // bottom right

    // Gradient colors
    ImVec4 color1 = ImVec4(102.0f / 255.0f, 51.0f / 255.0f, 102.0f / 255.0f, 1.0f);
    ImVec4 color2 = ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),  // top left to top right
        ImColor(color2), ImColor(color2)); // bottom left to bottom right

    // UI on top
    ImGui::SetCursorScreenPos(p0); 
    // Input selection
    ImGui::Text("Input Selection:");
    if (ImGui::BeginCombo("Input Image##Edge", 
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

    // Edge detection settings
    ImGui::Text("Edge Detection Settings");

    if (ImGui::Checkbox("Use Canny (else Sobel)", &useCanny)) {
        markDirty();
    }

    if (ImGui::Checkbox("Overlay Edges", &overlayEdges)) {
        markDirty();
    }

    if (useCanny) {
        // Canny parameters
        ImGui::Text("Canny Parameters:");
        if (ImGui::SliderInt("Threshold 1", &cannyThreshold1, 0, 255)) {
            markDirty();
        }
        if (ImGui::SliderInt("Threshold 2", &cannyThreshold2, 0, 255)) {
            markDirty();
        }
        if (ImGui::SliderInt("Aperture", &cannyAperture, 3, 7, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            cannyAperture = (cannyAperture / 2) * 2 + 1; // Ensure odd number
            markDirty();
        }
    } else {
        // Sobel parameters
        ImGui::Text("Sobel Parameters:");
        if (ImGui::Checkbox("Detect X", &sobelX)) {
            markDirty();
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("Detect Y", &sobelY)) {
            markDirty();
        }
        if (ImGui::SliderInt("Kernel Size", &sobelKSize, 1, 7, "%d", ImGuiSliderFlags_AlwaysClamp)) {
            sobelKSize = (sobelKSize / 2) * 2 + 1; // Ensure odd number
            markDirty();
        }
        if (ImGui::SliderFloat("Scale", &sobelScale, 0.1f, 5.0f)) {
            markDirty();
        }
        if (ImGui::SliderFloat("Delta", &sobelDelta, -5.0f, 5.0f)) {
            markDirty();
        }
    }

    // Display result
    if (texture) {
        ImGui::Text("Result:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat EdgeDetectionNode::getOutput() const {
    return output;
}

GLuint EdgeDetectionNode::matToTexture(const cv::Mat& mat) {
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
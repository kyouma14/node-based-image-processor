#include "BlurNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

BlurNode::BlurNode(int id) : Node(id, "Blur") {
    inputs.resize(1);  // One input for image
    radius = 5;
    directionalBlur = false;
    angle = 0.0f;
    texture = 0;
    kernelTexture = 0;
}

void BlurNode::process() {
    std::cout << "Processing Blur Node..." << std::endl;
    
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            // Create kernel based on current settings
            cv::Mat kernel;
            if (directionalBlur) {
                kernel = createDirectionalKernel(2 * radius + 1, angle);
            } else {
                kernel = createGaussianKernel(2 * radius + 1, radius/3.0);
            }

            // Apply filter
            cv::filter2D(input, output, -1, kernel);

            // Update textures
            if (texture) {
                glDeleteTextures(1, &texture);
            }
            texture = matToTexture(output);

            // Update kernel preview
            updateKernelPreview();
        } else {
            output = cv::Mat();  // Clear output if input is empty
        }
    } else {
        output = cv::Mat();  // Clear output if no input is connected
    }
    
    dirty = false;
}

void BlurNode::drawUI() {
    // Input selection
    ImGui::Text("Input Selection:");
    if (ImGui::BeginCombo("Input Image##Blur", 
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

    // Blur controls
    ImGui::Text("Blur Settings");
    
    if (ImGui::SliderInt("Radius", &radius, 1, 20)) {
        markDirty();
    }

    if (ImGui::Checkbox("Directional Blur", &directionalBlur)) {
        markDirty();
    }

    if (directionalBlur) {
        if (ImGui::SliderFloat("Angle", &angle, 0.0f, 360.0f)) {
            markDirty();
        }
    }

    // Display kernel preview
    if (kernelTexture) {
        ImGui::Text("Kernel Preview:");
        ImGui::Image((ImTextureID)(intptr_t)kernelTexture, ImVec2(100, 100));
    }

    // Display result preview
    if (texture) {
        ImGui::Text("Result Preview:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat BlurNode::getOutput() const {
    return output;
}

cv::Mat BlurNode::createGaussianKernel(int size, double sigma) {
    cv::Mat kernel = cv::getGaussianKernel(size, sigma);
    return kernel * kernel.t();  // Make 2D kernel
}

cv::Mat BlurNode::createDirectionalKernel(int size, float angle) {
    cv::Mat kernel = cv::Mat::zeros(size, size, CV_32F);
    float angleRad = angle * CV_PI / 180.0f;
    float dx = cos(angleRad);
    float dy = sin(angleRad);
    
    int center = size / 2;
    for (int i = 0; i < size; i++) {
        int y = i - center;
        for (int j = 0; j < size; j++) {
            int x = j - center;
            float dist = abs(x * dy - y * dx) / sqrt(dx * dx + dy * dy);
            kernel.at<float>(i, j) = exp(-dist * dist / 2.0f);
        }
    }
    
    // Normalize kernel
    kernel = kernel / cv::sum(kernel)[0];
    return kernel;
}

void BlurNode::updateKernelPreview() {
    cv::Mat kernel;
    if (directionalBlur) {
        kernel = createDirectionalKernel(2 * radius + 1, angle);
    } else {
        kernel = createGaussianKernel(2 * radius + 1, radius/3.0);
    }
    
    // Normalize for display
    cv::Mat kernelDisplay;
    cv::normalize(kernel, kernelDisplay, 0, 255, cv::NORM_MINMAX);
    kernelDisplay.convertTo(kernelDisplay, CV_8U);
    
    // Convert to RGB
    cv::Mat kernelRGB;
    cv::cvtColor(kernelDisplay, kernelRGB, cv::COLOR_GRAY2RGB);
    
    // Update texture
    if (kernelTexture) {
        glDeleteTextures(1, &kernelTexture);
    }
    kernelTexture = matToTexture(kernelRGB);
}

GLuint BlurNode::matToTexture(const cv::Mat& mat) {
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
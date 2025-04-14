#include "OutputNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>
#include "tinyfiledialogs.h"

OutputNode::OutputNode(int id) : Node(id, "Output") {
    inputs.resize(1);
}

void OutputNode::process() {
    if (inputs[0]) {
        output = inputs[0]->getOutput();
        
        if (!output.empty()) {
            if (texture) {
                glDeleteTextures(1, &texture);
            }
            texture = matToTexture(output);
        }
    }
    dirty = false;
}

void OutputNode::showSaveFileDialog() {
    const char* filters[3] = { "*.jpg", "*.png", "*.bmp" };
    const char* selected = tinyfd_saveFileDialog(
        "Save Image",
        "",
        3,
        filters,
        format == 0 ? "JPEG Files" : 
        format == 1 ? "PNG Files" : "BMP Files"
    );

    if (selected) {
        savePath = selected;
        saveImage();
    }
}

bool OutputNode::saveImage() {
    if (output.empty() || savePath.empty()) {
        return false;
    }

    std::vector<int> params;
    switch (format) {
        case 0: // JPG
            params.push_back(cv::IMWRITE_JPEG_QUALITY);
            params.push_back(jpgQuality);
            break;
        case 1: // PNG
            params.push_back(cv::IMWRITE_PNG_COMPRESSION);
            params.push_back(pngCompression);
            break;
        case 2: // BMP
            break;
    }

    try {
        return cv::imwrite(savePath, output, params);
    }
    catch (const cv::Exception& ex) {
        std::cerr << "Error saving image: " << ex.what() << std::endl;
        return false;
    }
}


void OutputNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();  // top left of content region
    ImVec2 size = ImGui::GetContentRegionAvail();  // drawable area
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);  // bottom right

    // Gradient colors
    ImVec4 color1 = ImVec4(54.0f / 255.0f, 69.0f / 255.0f, 79.0f / 255.0f, 1.0f);
    ImVec4 color2 = ImVec4(25.0f / 255.0f, 25.0f / 255.0f, 112.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),  // top left to top right
        ImColor(color2), ImColor(color2)); // bottom left to bottom right

    // UI on top
    ImGui::SetCursorScreenPos(p0); 
    // Input selection
    ImGui::Text("Input Image:");
    if (ImGui::BeginCombo("Input##Output", 
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

    // Format selection
    const char* formats[] = { "JPEG", "PNG", "BMP" };
    if (ImGui::Combo("Format", &format, formats, IM_ARRAYSIZE(formats))) {
        markDirty();
    }

    // Quality settings based on format
    switch (format) {
        case 0: // JPG
            if (ImGui::SliderInt("JPEG Quality", &jpgQuality, 0, 100)) {
                markDirty();
            }
            break;
        case 1: // PNG
            if (ImGui::SliderInt("PNG Compression", &pngCompression, 0, 9)) {
                markDirty();
            }
            break;
    }

    // Save button
    if (ImGui::Button("Save Image")) {
        showSaveFileDialog();
    }

    // Show save path if exists
    if (!savePath.empty()) {
        ImGui::Text("Last Save: %s", savePath.c_str());
    }

    // Preview
    if (texture) {
        ImGui::Text("Preview:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat OutputNode::getOutput() const {
    return output;
}

GLuint OutputNode::matToTexture(const cv::Mat& mat) {
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
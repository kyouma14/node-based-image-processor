#include "NoiseGenerationNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

NoiseGenerationNode::NoiseGenerationNode(int id) : Node(id, "Noise Generation") {
    inputs.resize(1);
    initPermutationTable();
}

void NoiseGenerationNode::initPermutationTable() {
    p.resize(512);
    std::iota(p.begin(), p.begin() + 256, 0);
    
    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.begin() + 256, rng);
    
    std::copy_n(p.begin(), 256, p.begin() + 256);
}

void NoiseGenerationNode::process() {
    if (inputs[0]) {
        cv::Mat input = inputs[0]->getOutput();
        if (!input.empty()) {
            // Update dimensions to match input image
            width = input.cols;
            height = input.rows;

            // Generate noise map
            cv::Mat noiseMap;
            switch (noiseType) {
                case 0:
                    noiseMap = generatePerlinNoise();
                    break;
                case 1:
                    noiseMap = generateSimplexNoise();
                    break;
                case 2:
                    noiseMap = generateWorleyNoise();
                    break;
            }

            if (useAsDisplacement) {
                // Use noise as displacement map
                output = applyDisplacementMap(input, noiseMap);
            } else {
                // Use noise as direct color addition
                cv::Mat processedNoise;
                if (input.channels() == 3) {
                    cv::cvtColor(noiseMap, processedNoise, cv::COLOR_GRAY2BGR);
                } else {
                    processedNoise = noiseMap;
                }
                processedNoise.convertTo(processedNoise, input.type());
                
                // Add noise to input image
                output = input.clone();
                cv::addWeighted(input, 1.0, processedNoise, noiseStrength, 0.0, output);
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

cv::Mat NoiseGenerationNode::applyDisplacementMap(const cv::Mat& input, const cv::Mat& noiseMap) {
    cv::Mat output = input.clone();
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Get displacement from noise
            float dx = (noiseMap.at<uchar>(y, x) / 255.0f - 0.5f) * displacementStrength;
            float dy = (noiseMap.at<uchar>(y, x) / 255.0f - 0.5f) * displacementStrength;
            
            // Calculate source pixel coordinates
            int sx = cv::borderInterpolate(x + static_cast<int>(dx), width, cv::BORDER_REFLECT_101);
            int sy = cv::borderInterpolate(y + static_cast<int>(dy), height, cv::BORDER_REFLECT_101);
            
            // Copy pixel
            if (input.channels() == 3) {
                output.at<cv::Vec3b>(y, x) = input.at<cv::Vec3b>(sy, sx);
            } else {
                output.at<uchar>(y, x) = input.at<uchar>(sy, sx);
            }
        }
    }
    
    return output;
}

cv::Mat NoiseGenerationNode::generatePerlinNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float nx = x / scale;
            float ny = y / scale;
            
            float value = octaveNoise(nx, ny);
            value = (value + 1.0f) * 0.5f;
            noiseMap.at<uchar>(y, x) = static_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

cv::Mat NoiseGenerationNode::generateSimplexNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    
    const float F2 = 0.5f * (sqrt(3.0f) - 1.0f);
    const float G2 = (3.0f - sqrt(3.0f)) / 6.0f;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float nx = x / scale;
            float ny = y / scale;
            
            float s = (nx + ny) * F2;
            int i = floor(nx + s);
            int j = floor(ny + s);
            
            float t = (i + j) * G2;
            float X0 = i - t;
            float Y0 = j - t;
            float x0 = nx - X0;
            float y0 = ny - Y0;
            
            int i1, j1;
            if (x0 > y0) {
                i1 = 1;
                j1 = 0;
            } else {
                i1 = 0;
                j1 = 1;
            }
            
            float x1 = x0 - i1 + G2;
            float y1 = y0 - j1 + G2;
            float x2 = x0 - 1.0f + 2.0f * G2;
            float y2 = y0 - 1.0f + 2.0f * G2;
            
            float n0 = simplexCornerNoise(x0, y0, i, j);
            float n1 = simplexCornerNoise(x1, y1, i + i1, j + j1);
            float n2 = simplexCornerNoise(x2, y2, i + 1, j + 1);
            
            float value = 70.0f * (n0 + n1 + n2);
            value = (value + 1.0f) * 0.5f;
            noiseMap.at<uchar>(y, x) = cv::saturate_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

cv::Mat NoiseGenerationNode::generateWorleyNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    std::vector<cv::Point2f> points;
    
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    int numPoints = 20;
    for (int i = 0; i < numPoints; i++) {
        points.push_back(cv::Point2f(
            dist(rng) * width,
            dist(rng) * height
        ));
    }
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float minDist = FLT_MAX;
            float secondMinDist = FLT_MAX;
            
            for (const auto& point : points) {
                float dx = x - point.x;
                float dy = y - point.y;
                float dist = sqrt(dx * dx + dy * dy);
                
                if (dist < minDist) {
                    secondMinDist = minDist;
                    minDist = dist;
                } else if (dist < secondMinDist) {
                    secondMinDist = dist;
                }
            }
            
            float value = (secondMinDist - minDist) / scale;
            noiseMap.at<uchar>(y, x) = cv::saturate_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

float NoiseGenerationNode::simplexCornerNoise(float x, float y, int i, int j) {
    float t = 0.5f - x * x - y * y;
    if (t < 0.0f) return 0.0f;
    
    int gi = (p[(i + p[j & 255]) & 255]) % 12;
    t *= t;
    return t * t * grad(gi, x, y);
}

float NoiseGenerationNode::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float NoiseGenerationNode::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float NoiseGenerationNode::grad(int hash, float x, float y) {
    int h = hash & 15;
    float grad_x = 1 + (h & 7);
    if (h & 8) grad_x = -grad_x;
    float grad_y = 1 + (h & 7);
    if (h & 8) grad_y = -grad_y;
    return grad_x * x + grad_y * y;
}

float NoiseGenerationNode::noise2D(float x, float y) {
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    
    x -= std::floor(x);
    y -= std::floor(y);
    
    float u = fade(x);
    float v = fade(y);
    
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];
    
    return lerp(
        lerp(grad(p[AA], x, y),
             grad(p[BA], x - 1, y),
             u),
        lerp(grad(p[AB], x, y - 1),
             grad(p[BB], x - 1, y - 1),
             u),
        v);
}

float NoiseGenerationNode::octaveNoise(float x, float y) {
    float total = 0;
    float frequency = 1;
    float amplitude = 1;
    float maxValue = 0;
    
    for (int i = 0; i < octaves; i++) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }
    
    return total / maxValue;
}

void NoiseGenerationNode::drawUI() {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p0 = ImGui::GetCursorScreenPos();
    ImVec2 size = ImGui::GetContentRegionAvail();
    ImVec2 p1 = ImVec2(p0.x + size.x, p0.y + size.y);

    // Gradient background
    ImVec4 color1 = ImVec4(84.0f / 255.0f, 90.0f / 255.0f, 0.0f / 255.0f, 1.0f);
    ImVec4 color2 = ImVec4(0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f);

    draw_list->AddRectFilledMultiColor(p0, p1,
        ImColor(color1), ImColor(color1),
        ImColor(color2), ImColor(color2));

    // UI on top
    ImGui::SetCursorScreenPos(p0);
    
    // Input selection
    ImGui::Text("Input Image:");
    if (ImGui::BeginCombo("Input##Noise", 
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

    // Noise parameters
    const char* noiseTypes[] = { "Perlin", "Simplex", "Worley" };
    if (ImGui::Combo("Noise Type", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes))) {
        markDirty();
    }

    if (ImGui::Checkbox("Use as Displacement Map", &useAsDisplacement)) {
        markDirty();
    }

    if (useAsDisplacement) {
        if (ImGui::SliderFloat("Displacement Strength", &displacementStrength, 0.0f, 50.0f)) {
            markDirty();
        }
    } else {
        if (ImGui::SliderFloat("Noise Strength", &noiseStrength, 0.0f, 1.0f)) {
            markDirty();
        }
    }

    if (ImGui::SliderFloat("Scale", &scale, 1.0f, 100.0f)) {
        markDirty();
    }

    if (ImGui::SliderInt("Octaves", &octaves, 1, 8)) {
        markDirty();
    }

    if (ImGui::SliderFloat("Persistence", &persistence, 0.0f, 1.0f)) {
        markDirty();
    }

    if (ImGui::SliderFloat("Lacunarity", &lacunarity, 1.0f, 4.0f)) {
        markDirty();
    }

    if (ImGui::InputInt("Seed", &seed)) {
        initPermutationTable();
        markDirty();
    }

    // Result preview
    if (texture) {
        ImGui::Text("Result:");
        ImGui::Image((ImTextureID)(intptr_t)texture, ImVec2(300, 300));
    }
}

cv::Mat NoiseGenerationNode::getOutput() const {
    return output;
}

GLuint NoiseGenerationNode::matToTexture(const cv::Mat& mat) {
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

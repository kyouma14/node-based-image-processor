#include "NoiseGenerationNode.h"
#include <imgui.h>
#include <opencv2/imgproc.hpp>

NoiseGenerationNode::NoiseGenerationNode(int id) : Node(id, "Noise Generation") {
    initPermutationTable();
}

void NoiseGenerationNode::initPermutationTable() {
    // Initialize permutation table with values 0-255
    p.resize(512);
    std::iota(p.begin(), p.begin() + 256, 0);
    
    // Shuffle first 256 elements
    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.begin() + 256, rng);
    
    // Duplicate the array
    std::copy_n(p.begin(), 256, p.begin() + 256);
}

void NoiseGenerationNode::process() {
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

    if (colorOutput) {
        // Convert noise to color
        cv::Mat colorNoise;
        cv::applyColorMap(noiseMap, colorNoise, cv::COLORMAP_JET);
        output = colorNoise;
    } else {
        output = noiseMap;
    }

    if (texture) {
        glDeleteTextures(1, &texture);
    }
    texture = matToTexture(output);
    
    dirty = false;
}

cv::Mat NoiseGenerationNode::generatePerlinNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float nx = x / scale;
            float ny = y / scale;
            
            float value = octaveNoise(nx, ny);
            
            // Normalize to 0-1 range
            value = (value + 1.0f) * 0.5f;
            // Convert to 0-255 range
            noiseMap.at<uchar>(y, x) = static_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

cv::Mat NoiseGenerationNode::generateSimplexNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    
    // Simplex noise constants
    const float F2 = 0.5f * (sqrt(3.0f) - 1.0f);
    const float G2 = (3.0f - sqrt(3.0f)) / 6.0f;
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float nx = x / scale;
            float ny = y / scale;
            
            // Skew input space to determine which simplex cell we're in
            float s = (nx + ny) * F2;
            int i = floor(nx + s);
            int j = floor(ny + s);
            
            // Unskew back to (x,y) space
            float t = (i + j) * G2;
            float X0 = i - t;
            float Y0 = j - t;
            float x0 = nx - X0;
            float y0 = ny - Y0;
            
            // Determine which simplex we're in
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
            
            // Calculate noise contributions from each corner
            float n0 = simplexCornerNoise(x0, y0, i, j);
            float n1 = simplexCornerNoise(x1, y1, i + i1, j + j1);
            float n2 = simplexCornerNoise(x2, y2, i + 1, j + 1);
            
            // Add contributions and scale to [-1, 1]
            float value = 70.0f * (n0 + n1 + n2);
            
            // Scale to [0, 255]
            value = (value + 1.0f) * 0.5f;
            noiseMap.at<uchar>(y, x) = cv::saturate_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

float NoiseGenerationNode::simplexCornerNoise(float x, float y, int i, int j) {
    float t = 0.5f - x * x - y * y;
    if (t < 0.0f) return 0.0f;
    
    // Hash coordinates
    int gi = (p[(i + p[j & 255]) & 255]) % 12;
    
    // Calculate gradient contribution
    t *= t;
    return t * t * grad(gi, x, y);
}

cv::Mat NoiseGenerationNode::generateWorleyNoise() {
    cv::Mat noiseMap(height, width, CV_8UC1);
    std::vector<cv::Point2f> points;
    
    // Generate random feature points
    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    // Number of feature points
    int numPoints = 20;
    for (int i = 0; i < numPoints; i++) {
        points.push_back(cv::Point2f(
            dist(rng) * width,
            dist(rng) * height
        ));
    }
    
    // Calculate distances for each pixel
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float minDist = FLT_MAX;
            float secondMinDist = FLT_MAX;
            
            // Find closest feature point
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
            
            // Use F2-F1 difference for interesting patterns
            float value = (secondMinDist - minDist) / scale;
            noiseMap.at<uchar>(y, x) = cv::saturate_cast<uchar>(value * 255);
        }
    }
    
    return noiseMap;
}

float NoiseGenerationNode::fade(float t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

float NoiseGenerationNode::lerp(float a, float b, float t) {
    return a + t * (b - a);
}

float NoiseGenerationNode::grad(int hash, float x, float y) {
    int h = hash & 15;
    float grad_x = 1 + (h & 7);  // Gradient value for x
    if (h & 8) grad_x = -grad_x; // Random sign
    float grad_y = 1 + (h & 7);  // Gradient value for y
    if (h & 8) grad_y = -grad_y; // Random sign
    return grad_x * x + grad_y * y;
}

float NoiseGenerationNode::noise2D(float x, float y) {
    // Find unit cube that contains point
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    
    // Find relative x, y of point in cube
    x -= std::floor(x);
    y -= std::floor(y);
    
    // Compute fade curves for each of x, y
    float u = fade(x);
    float v = fade(y);
    
    // Hash coordinates of the 4 cube corners
    int A = p[X] + Y;
    int AA = p[A];
    int AB = p[A + 1];
    int B = p[X + 1] + Y;
    int BA = p[B];
    int BB = p[B + 1];
    
    // Add blended results from 4 corners of cube
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
    ImGui::Text("Noise Generation Settings");

    const char* noiseTypes[] = { "Perlin", "Simplex", "Worley" };
    if (ImGui::Combo("Noise Type", &noiseType, noiseTypes, IM_ARRAYSIZE(noiseTypes))) {
        markDirty();
    }

    if (ImGui::SliderInt("Width", &width, 64, 1024)) {
        markDirty();
    }

    if (ImGui::SliderInt("Height", &height, 64, 1024)) {
        markDirty();
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

    if (ImGui::Checkbox("Color Output", &colorOutput)) {
        markDirty();
    }

    if (texture) {
        ImGui::Text("Preview:");
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
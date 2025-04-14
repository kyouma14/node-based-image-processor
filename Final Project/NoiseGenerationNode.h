#pragma once
#include "Node.h"
#include <GL/glew.h>
#include <random>
#include <numeric>

class NoiseGenerationNode : public Node {
public:
    NoiseGenerationNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;

private:
    cv::Mat output;
    cv::Mat inputImage;  // Added to store input image
    GLuint texture = 0;
    
    // Noise parameters
    int noiseType = 0;  // 0: Perlin, 1: Simplex, 2: Worley
    int width = 512;
    int height = 512;
    float scale = 50.0f;
    int octaves = 4;
    float persistence = 0.5f;
    float lacunarity = 2.0f;
    int seed = 1234;
    bool colorOutput = false;  // false for displacement map, true for color
    float noiseStrength = 0.5f;  // Added to control noise intensity
    bool addToInput = true;  // Added to toggle between noise generation and addition
    
    // Noise generation methods
    cv::Mat generatePerlinNoise();
    cv::Mat generateSimplexNoise();
    cv::Mat generateWorleyNoise();
    float simplexCornerNoise(float x, float y, int i, int j);
    
    // Helper methods
    GLuint matToTexture(const cv::Mat& mat);
    float fade(float t);
    float lerp(float a, float b, float t);
    float grad(int hash, float x, float y);
    float noise2D(float x, float y);
    float octaveNoise(float x, float y);
    
    // Added helper method for adding noise to input
    cv::Mat addNoiseToImage(const cv::Mat& input, const cv::Mat& noise);
    
    // Permutation table for Perlin noise
    std::vector<int> p;
    void initPermutationTable();
};
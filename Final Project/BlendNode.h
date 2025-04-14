#pragma once
#include "Node.h"
#include <GL/glew.h>

class BlendNode : public Node {
public:
    BlendNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    
    void drawUI() override;



private:
    
    cv::Mat output;
    cv::Mat secondImage;  // For the directly loaded image
    std::string secondImagePath;
    GLuint texture = 0;
    GLuint secondImageTexture = 0;  // Texture for preview of second image
    
    // Blend parameters
    float opacity = 1.0f;
    int blendMode = 0;  // 0: Normal, 1: Multiply, 2: Screen, 3: Overlay, 4: Difference

    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    cv::Mat applyBlend(const cv::Mat& base, const cv::Mat& blend, int mode, float opacity);
    cv::Mat multiplyBlend(const cv::Mat& base, const cv::Mat& blend);
    cv::Mat screenBlend(const cv::Mat& base, const cv::Mat& blend);
    cv::Mat overlayBlend(const cv::Mat& base, const cv::Mat& blend);
    cv::Mat differenceBlend(const cv::Mat& base, const cv::Mat& blend);
};
#pragma once
#include "Node.h"
#include <GL/glew.h>

class BlurNode : public Node {
public:
    BlurNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;



private:
    cv::Mat output;
    GLuint texture = 0;
    GLuint kernelTexture = 0;  // For displaying the kernel

    // Blur parameters
    int radius = 5;            // 1-20px
    bool directionalBlur = false;
    float angle = 0.0f;        // For directional blur
    
    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    void updateKernelPreview();
    cv::Mat createGaussianKernel(int size, double sigma);
    cv::Mat createDirectionalKernel(int size, float angle);
};
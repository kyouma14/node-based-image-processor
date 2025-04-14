#pragma once
#include "Node.h"
#include <GL/glew.h>

class ConvolutionFilterNode : public Node {
public:
    ConvolutionFilterNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;

private:
    cv::Mat output;
    GLuint texture = 0;
    GLuint previewTexture = 0;  // For kernel effect preview

    // Kernel parameters
    static const int MAX_KERNEL_SIZE = 5;
    int kernelSize = 3;  // 3x3 or 5x5
    std::vector<float> kernel;
    float kernelDivisor = 1.0f;
    float kernelOffset = 0.0f;

    // Preset filters
    enum class Preset {
        Custom,
        Sharpen,
        Emboss,
        EdgeEnhance,
        BoxBlur,
        GaussianBlur,
        EdgeDetect
    };
    Preset currentPreset = Preset::Custom;

    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    void applyPreset(Preset preset);
    cv::Mat applyKernel(const cv::Mat& input);
    void updatePreview();
    void resetKernel();
};
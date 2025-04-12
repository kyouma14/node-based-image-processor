#pragma once
#include "Node.h"
#include <GL/glew.h>

class ThresholdNode : public Node {
public:
    ThresholdNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;

private:
    cv::Mat output;
    GLuint texture = 0;
    GLuint histogramTexture = 0;

    // Threshold parameters
    int thresholdValue = 127;
    int maxValue = 255;
    int thresholdType = cv::THRESH_BINARY;
    bool useOtsu = false;
    bool useAdaptive = false;
    int adaptiveMethod = cv::ADAPTIVE_THRESH_MEAN_C;
    int blockSize = 11;
    double C = 2.0;

    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    void updateHistogram(const cv::Mat& input);
    void drawHistogram();
};
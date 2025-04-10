#pragma once
#include "Node.h"
#include <GL/glew.h>

class BrightnessContrastNode : public Node {
public:
    float brightness = 0.0f;  // Range: -100 to 100
    float contrast = 1.0f;    // Range: 0 to 3
    cv::Mat output;
    GLuint texture = 0;

    BrightnessContrastNode(int id);

    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;

private:
    GLuint matToTexture(const cv::Mat& mat);
};

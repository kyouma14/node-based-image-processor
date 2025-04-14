#pragma once
#include "Node.h"
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

class ColorChannelSplitNode : public Node {
public:
    cv::Mat redChannel, greenChannel, blueChannel, alphaChannel;
    GLuint redTex = 0, greenTex = 0, blueTex = 0, alphaTex = 0;
    bool grayscale = true;
    GLuint textures[4] = {0};
    int selectedChannel = 0;

    ColorChannelSplitNode(int id);

    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;
    


private:
    GLuint matToTexture(const cv::Mat& mat);
};
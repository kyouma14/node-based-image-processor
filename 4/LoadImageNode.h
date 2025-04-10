#pragma once
#include "Node.h"
#include <string>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

class LoadImageNode : public Node {
public:
    LoadImageNode(int id);

    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;

private:
    std::string filePath;
    cv::Mat image;
    GLuint texture = 0;

    GLuint matToTexture(const cv::Mat& mat);
};
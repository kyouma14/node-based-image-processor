#pragma once
#include "Node.h"
#include <string>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

class LoadImageNode : public Node {
public:
    LoadImageNode(int id);
    void drawUI() override;
    void process() override;
    cv::Mat getOutput() const override;
    


private:
    std::string filePath;
    cv::Mat image;
    GLuint texture = 0;
    GLuint matToTexture(const cv::Mat& mat);
    char filepath[256] = "";
    GLuint textureID;
};
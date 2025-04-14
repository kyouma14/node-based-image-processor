#pragma once
#include "Node.h"
#include <GL/glew.h>

class OutputNode : public Node {
public:
    OutputNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;


private:
    cv::Mat output;
    GLuint texture = 0;
    
    // Output parameters
    std::string savePath;
    int format = 0;  // 0: JPG, 1: PNG, 2: BMP
    int jpgQuality = 95;  // 0-100 for JPG
    int pngCompression = 6;  // 0-9 for PNG
    
    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    bool saveImage();
    void showSaveFileDialog();
};
#pragma once
#include "Node.h"
#include <GL/glew.h>

class EdgeDetectionNode : public Node {
public:
    EdgeDetectionNode(int id);
    void process() override;
    cv::Mat getOutput() const override;
    void drawUI() override;



private:
    cv::Mat output;
    cv::Mat overlayOutput;  // For edge overlay on original
    GLuint texture = 0;
    
    // Edge detection parameters
    bool useCanny = true;
    bool overlayEdges = false;
    
    // Canny parameters
    int cannyThreshold1 = 100;
    int cannyThreshold2 = 200;
    int cannyAperture = 3;
    
    // Sobel parameters
    int sobelKSize = 3;
    float sobelScale = 1.0f;
    float sobelDelta = 0.0f;
    bool sobelX = true;
    bool sobelY = true;
    
    // Methods
    GLuint matToTexture(const cv::Mat& mat);
    cv::Mat applySobel(const cv::Mat& input);
    cv::Mat applyCanny(const cv::Mat& input);
    cv::Mat createOverlay(const cv::Mat& original, const cv::Mat& edges);
};
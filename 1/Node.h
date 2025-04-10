// Node.h
#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

class Node {
public:
    int id;
    std::string name;
    std::vector<Node*> inputs;
    std::vector<Node*> outputs;
    bool dirty = true; // Marks if output needs recomputation

    Node(int id, const std::string& name) : id(id), name(name) {}

    virtual ~Node() = default;

    // Run the processing logic
    virtual void process() = 0;

    // Get the output image
    virtual cv::Mat getOutput() const = 0;

    // Draw the node's UI (ImGui widgets)
    virtual void drawUI() = 0;

    // Optional: connect inputs
    virtual void setInput(int index, Node* node) {
        if (index < inputs.size()) {
            inputs[index] = node;
        }
    }

    // Called when parameters or inputs change
    virtual void markDirty() {
        dirty = true;
        for (auto* output : outputs) {
            if (output) output->markDirty();
        }
    }
};

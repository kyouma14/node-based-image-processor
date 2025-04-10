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
    bool dirty = true;

    static std::vector<Node*> availableNodes;

    Node(int id, const std::string& name) : id(id), name(name) {}
    virtual ~Node() = default;

    virtual void process() = 0;
    virtual cv::Mat getOutput() const = 0;
    virtual void drawUI() = 0;
    
    virtual const std::string& getName() const { return name; }

    virtual void setInput(int index, Node* node) {
        if (index < inputs.size()) {
            inputs[index] = node;
            markDirty();
        }
    }

    virtual void markDirty() {
        dirty = true;
        for (auto* output : outputs) {
            if (output) output->markDirty();
        }
    }

    static void registerNode(Node* node) {
        availableNodes.push_back(node);
    }
    
    static void clearNodes() {
        availableNodes.clear();
    }
};

inline std::vector<Node*> Node::availableNodes;
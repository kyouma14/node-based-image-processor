cmake_minimum_required(VERSION 3.15)
project(NodeEditor)  


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_TOOLCHAIN_FILE "D:/Mixar/vcpkg/scripts/buildsystems/vcpkg.cmake" )

# Find OpenCV
find_package(OpenCV REQUIRED)


#find imgui

find_package(imgui CONFIG REQUIRED)

#find glfw3

find_package(glfw3 CONFIG REQUIRED)

#find glew

find_package(glew CONFIG REQUIRED)



add_executable(
    NodeEditor main1.cpp
    tinyfiledialogs.c
    LoadImageNode.cpp
    BrightnessContrastNode.cpp
    ColorChannelSplitter.cpp
    blurnode.cpp
    ThresholdNode.cpp
    EdgeDetectionNode.cpp
    BlendNode.cpp
    NoiseGenerationNode.cpp
    ConvolutionFilterNode.cpp
)



# target_include_directories(NodeEditor PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(
    NodeEditor PRIVATE
    ${OpenCV_LIBS}
    imgui::imgui
    glfw
    glew32
    opengl32
)
target_link_directories(NodeEditor
    PRIVATE
        "D:/Mixar/vcpkg/installed/x64-windows/lib"
)



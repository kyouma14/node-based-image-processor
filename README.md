# node-based-image-processor

## Project Overview
The project aims to implement a graph based image processing system using a node interface. Each node will carry out its own operation (eg. Blurring, Edge Detection, Brightness/Contrast, etc.) on an input that it will recieve from another node. This interface will allow the user to connect the nodes in any way they desire to perform image manipulation. 
The UI is designed using Dear ImGui, which was chosen due to its easy integration with OpenGL and GLFW. OpenCV is used for image processing operations. 

## Features
1: **Input Node**: This node loads an image from the disk.

![image](https://github.com/user-attachments/assets/951bafc7-2087-44b4-8a02-52ca4b1bd90f)


2: **Brightness/Contrast Node**: This node adjusts the brightness and/or contrast of the image with the help of sliders.

![image](https://github.com/user-attachments/assets/6f88e4c7-6aa2-4a1b-81f7-694dea0c93c4)


3: **Color Channel Splitter Node**: This nodes splits the input image into three color channels (Blue,Green and Red). It also gives the option to convert it into grayscale.

![image](https://github.com/user-attachments/assets/379837f0-a939-4f63-b27b-03345f92d3be)


4: **Blur Node**: This node implements Gaussian Blur with an adjustable radius of 1-20px using a slider. The blur can be switched between uniform and directional. It also includes a preview of the kernel as well.

![image](https://github.com/user-attachments/assets/2519c5db-2dc7-4850-b947-0c881e9f4528)


5: **Threshold Node**: This node converts the image into a binary image based on a threshold value set by the user in the node with a slider. The node accomodates different types of thresholding such as binary, adaptive and Otsu. A histogram of the image is also shown.

![image](https://github.com/user-attachments/assets/969a50a9-72b9-47d7-9596-a8f04ff18ae3)


6: **Edge Detection Node**: This node carries out edge detection in the input image. It implements either Sobel or Canny detection algorithm based on user preference, which can be selected via a checkbox. It also gives the user an option to overlay edges on the original image (via a checkbox as well). 

![image](https://github.com/user-attachments/assets/054746ab-ec93-4b5e-9d1b-b829c3f4911f)


7: **Blend Node**: This node combines two different images using different blend modes like normal, multiply, screen, overlay etc. The first image is taken as input from the node system. The second image is taken from the disk by the node. The node also has an opacity slider.

![image](https://github.com/user-attachments/assets/c0f72f4e-19f0-47bd-830c-286f84d13642)


8: **Noise Generation Node**: This node generates random noise in the input image. These can be of the patterns Perlin, Simplex or Worley. There are also sliders provided for configuration of various noise parameters such as scales, octaves, persistence, lacunarity etc. 

![image](https://github.com/user-attachments/assets/294eb17e-f4d4-41d6-9f9f-fbae2b619ea5)


9: **Convolution Filter Node**: This node provides a custom 3x3 or 5x5 matrix (selectable by a checkbox) for various custom convolutions. It also comes with presets such as sharpen, emboss, edge enhancement etc. 

![image](https://github.com/user-attachments/assets/abdc39ee-8eaa-40a5-9220-6f4a37baca12)


10: **Output Node**: This node previews the final image and allows the user to select in which format they want to save it (Jpeg, PNG, BMP). It also includes a slider which can be adjusted to increase or decrease the quality of the image being saved. Finally it gives an option of saving the image which would then ask the users to select the path where they want to save the image. 

![image](https://github.com/user-attachments/assets/00e5f89f-4e4e-4d09-9188-92bc21f1f6f0)

## Build instructions

### Prerequisites:
1: CMake

2: vcpkg (https://github.com/microsoft/vcpkg) (for dependency management)

3: compiler like MSVC

### Dependencies:
Run these commands in the directory where vcpkg.cmake exists. Usually, it will be "external/vcpkg/scripts/buildsystems". If vcpkg is set to path, it can be run from anywhere. 

ImGui : ./vcpkg install imgui[core,glfw-binding,opengl3-binding] 

GLFW : ./vcpkg install glfw3 

OpenCV : ./vcpkg install opencv4 

GLEW : ./vcpkg install glew 

Integrate CMake and vcpkg by running the following command:

./vcpkg integrate install

Then link the libraries in CMakeLists.txt

### Build
1: Go to the project directory.

cd "directory/project"

2: Create a folder "build":

mkdir build

3: Go to the folder:

cd build

4: The toolchain file is specified so that CMake correctly links all the dependencies managed by vcpkg:

cmake .. -DCMAKE_TOOLCHAIN_FILE="disk/vcpkg/scripts/buildsystems/vcpkg.cmake" -DCMAKE_BUILD_TYPE=Debug 

5: Then, for building the project:

cmake --build . --config Debug

The .exe file will be found in "project/build/debug"

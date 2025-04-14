# node-based-image-processor

## Project Overview
The project aims to implement a graph based image processing system using a node interface. Each node will carry out its own operation (eg. Blurring, Edge Detection, Brightness/Contrast, etc.) on an input that it will recieve from another node. This interface will allow the user to connect the nodes in any way they desire to perform image manipulation. 
The UI is designed using Dear ImGui, which was chosen due to its easy integration with OpenGL and GLFW. OpenCV is used for image processing operations. 

## Features
1: Input Node: This node loads an image from the disk.

2: Brightness/Contrast Node: This node adjusts the brightness and/or contrast of the image with the help of sliders.

3: Color Channel Splitter Node: This nodes splits the input image into three color channels (Blue,Green and Red). It also gives the option to convert it into grayscale.

4: Blur Node: This node implements Gaussian Blur with an adjustable radius of 1-20px using a slider. The blur can be switched between uniform and directional. It also includes a preview of the kernel as well. 

5: Threshold Node: This node converts the image into a binary image based on a threshold value set by the user in the node with a slider. The node accomodates different types of thresholding such as binary, adaptive and Otsu. A histogram of the image is also shown. 

6: Edge Detection Node: This node carries out edge detection in the input image. It implements either Sobel or Canny detection algorithm based on user preference, which can be selected via a checkbox. It also gives the user an option to overlay edges on the original image (via a checkbox as well). 

7: Blend Node: This node combines two different images using different blend modes like normal, multiply, screen, overlay etc. The first image is taken as input from the node system. The second image is taken from the disk by the node. The node also has an opacity slider.

8: Noise Generation Node: This node generates random noise in the input image. These can be of the patterns Perlin, Simplex or Worley. There are also sliders provided for configuration of various noise parameters such as scales, octaves, persistence, lacunarity etc. 

9: Convolution Filter Node: This node provides a custom 3x3 or 5x5 matrix (selectable by a checkbox) for various custom convolutions. It also comes with presets such as sharpen, emboss, edge enhancement etc. 

10: Output Node: This node previews the final image and allows the user to select in which format they want to save it (Jpeg, PNG, BMP). It also includes a slider which can be adjusted to increase or decrease the quality of the image being saved. Finally it gives an option of saving the image which would then ask the users to select the path where they want to save the image. 

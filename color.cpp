#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "rapidcsv.h"
#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#define width 640
#define length 480

//g++ -o color_detection color.cpp `pkg-config --cflags --libs opencv4` -std=c++11
int r,g,b;
// Structure to hold color information
struct ColorInfo {
    std::string name;
    int r, g, b;
};

//read colordata from csv file
std::vector<ColorInfo> loadColorData(const std::string& csvFile) {
    std::vector<ColorInfo> colorData;

    
        rapidcsv::Document doc(csvFile);
        for (size_t i = 0; i < doc.GetRowCount(); ++i) {
            ColorInfo color;
            color.name = doc.GetCell<std::string>("Name", i);
            color.r = doc.GetCell<int>("R", i);
            color.g = doc.GetCell<int>("G", i);
            color.b = doc.GetCell<int>("B", i);
            colorData.push_back(color);

            std::cout << "Loaded color: " << color.name << " ("
                      << color.r << ", " << color.g << ", " << color.b << ")" << std::endl;
        }

    return colorData;
}

//display function
void displayColorInfo(cv::Mat& frame, const std::string& colorName, const std::string& tone, int x, int y, int r, int g, int b) {
        
    cv::putText(frame, "RGB: " + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b),
                cv::Point(x + 10, y + 10), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);

    cv::putText(frame, "Color Name: " + colorName, cv::Point(x + 10, y + 30), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    cv::putText(frame, "Tone: " + tone, cv::Point(x + 10, y + 50), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
    cv::imshow("Color Detection", frame);
    
    cv::waitKey(2000); // freeze the image for 2 seconds
}


void clickAction(int event, int x, int y, int flags, void* userdata) {
    if (event != cv::EVENT_LBUTTONDOWN) {
        return;
    }

    auto* data = static_cast<std::pair<cv::Mat*, std::vector<ColorInfo>*>*>(userdata);
    if (!data || !data->first || !data->second) {
        std::cerr << "Invalid user data." << std::endl;
        return;
    }

    cv::Mat* frame = data->first;
    std::vector<ColorInfo>* colorData = data->second;

    if (frame->empty() || colorData->empty()) {
        std::cerr << "Frame or color data is empty." << std::endl;
        return;
    }

    cv::Vec3b rgb = frame->at<cv::Vec3b>(y, x);
    int b = rgb[0];
    int g = rgb[1];
    int r = rgb[2];

    std::cout << "RGB values at (" << x << ", " << y << "): " << r << ", " << g << ", " << b << std::endl;

    // determine if the color is a red tone or green tone
    std::string tone;
    if (128 <= r && r <= 255 && g <= 200 && b <= 200 && r > g && r > b) {
        tone = "Red Tone";
    } else if (r <= 200 && 128 <= g && g <= 255 && b <= 200 && g > r && g > b) {
        tone = "Green Tone";
    } else if (70 <= r && r <= 150 && 50 <= g && g <= 100 && b <= 75 && r > g && g > b) {
    tone = "Red-Brownish Tone";
    } else if (70 <= g && g <= 150 && 50 <= r && r <= 100 && b <= 75 && g > r && r > b) {
    tone = "Green-Brownish Tone";
    } else{
        tone = "Other";
    }

    // check the color name from CSV file
    std::string colorName = "No matching color found";
    for (const auto& color : *colorData) {
        if (std::abs(r - color.r) <= 20 && std::abs(g - color.g) <= 20 && std::abs(b - color.b) <= 20) {
            colorName = color.name;
            break;
        }
    }

    std::cout << "Color Name: " << colorName << ", Tone: " << tone << std::endl;


    // Set the color information for display
    std::chrono::steady_clock::time_point currenttime = std::chrono::steady_clock::now();
    auto currentTimeAfter = std::chrono::steady_clock::now();
    auto timeDifference = std::chrono::duration_cast<std::chrono::seconds>(currentTimeAfter - currenttime);

    // Check if less than 2 seconds have passed since currenttime
    if (timeDifference.count() < 2) {   
    displayColorInfo(*frame, colorName, tone, x, y, r, g, b);
    }
}

int main(int argc, char** argv) {
    // load color data from CSV file
    std::string csvFile = "color_names.csv";
    std::vector<ColorInfo> colorData = loadColorData(csvFile);

    // set camera
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open webcam." << std::endl;
        return -1;
    }

    //resolution
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, length);

    cv::namedWindow("Color Detection");
    cv::Mat frame;
    std::pair<cv::Mat*, std::vector<ColorInfo>*> data(&frame, &colorData);
    cv::setMouseCallback("Color Detection", clickAction, &data);

    while (true) {
        cap >> frame;

        
        // Display static text on the image
        cv::putText(frame, "Press 'q' to quit", cv::Point(10, 450), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
        cv::imshow("Color Detection", frame);

        //press q to quit
        if (cv::waitKey(1) == 'q') {
            break;
        }
    }

    cap.release();
    cv::destroyAllWindows();
    return 0;
}

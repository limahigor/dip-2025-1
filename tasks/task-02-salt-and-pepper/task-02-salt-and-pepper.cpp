#include <iostream>
#include <vector>
#include <random>
#include <numeric>
#include <opencv2/opencv.hpp>

cv::Mat create_salt_and_pepper_noise(int height, int width, double salt_prob, double pepper_prob) {
    cv::Mat img = cv::Mat::ones(height, width, CV_32FC1) * 0.5;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.0, 1.0);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            double random_value = dis(gen);
            if (random_value < salt_prob) {
                img.at<float>(i, j) = 1.0f;
            } else if (random_value > 1.0 - pepper_prob) {
                img.at<float>(i, j) = -1.0f;
            }
        }
    }
    return img;
}

int main() {
    int width = 100;
    int height = 100;
    double salt_prob = 0.1;
    double pepper_prob = 0.1;

    cv::Mat img = create_salt_and_pepper_noise(height, width, salt_prob, pepper_prob);

    int salt_count = 0;
    int pepper_count = 0;
    bool test_passed = true;

    for(int i = 0; i < width; i++){
        for (int j = 0; j < height; j++){
            if(std::abs(img.at<float>(i, j) - 1.0f) < 0.0001f){
                salt_count++;
            }else if (std::abs(img.at<float>(i, j) - (-1.0f)) < 0.0001f){
                pepper_count++;
            }
        }
    }

    if(salt_count <= 900 || salt_count >= 1100){
        std::cout << "Salt pixel count is outside expected range." << std::endl;
        test_passed = false;
    }
    
    if(pepper_count <= 900 || pepper_count >= 1100){
        std::cout << "Pepper pixel count is outside expected range." << std::endl;
        test_passed = false;
    }

    if(test_passed){
        std::cout << "Test passed!" << std::endl;
    }

    return 0;
}
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>

cv::Mat generate_image(int seed, int width, int height, double mean, double std) {
    std::mt19937 generator(seed);

    std::normal_distribution<double> distribution(mean, std);

    cv::Mat image(height, width, CV_32F);

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            image.at<float>(i, j) = distribution(generator);
        }
    }

    return image;
}

int main() {
    int seed = 0;
    int width = 3;
    int height = 5;
    float mean = 128.0f;
    float std = 20.0f;

    cv::Mat image = generate_image(seed, width, height, mean, std);

    cv::Mat test = (cv::Mat_<float>(5, 3) <<
        150.45590210f, 134.05610657f, 129.41719055f,
        129.46083069f, 99.55348206f, 158.40138245f,
        122.17211914f, 125.33819580f, 124.53860474f, 
        92.76696777f, 126.24653625f, 155.33758545f, 
        150.50628662f, 120.82009125f, 152.41215515f);

    cv::Mat diff;
    cv::absdiff(image, test, diff);
    cv::Mat nonZeroCount = diff > 0.0001f;

    if (cv::countNonZero(nonZeroCount) == 0) {
        std::cout << "Test passed!" << std::endl;
    } else {
        std::cout << "Test failed!" << std::endl;
    }

    return 0;
}
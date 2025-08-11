#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <filesystem>

float compute_histogram_intersection(cv::Mat img1, cv::Mat img2){
    int histSize = 256;
    float range[] = {0.f, 256.f};
    const float* ranges[] = {range};    

    cv::Mat h1, h2;
    
    cv::calcHist(&img1, 1, nullptr, cv::Mat(), h1, 1, &histSize, ranges, true, false);
    cv::calcHist(&img2, 1, nullptr, cv::Mat(), h2, 1, &histSize, ranges, true, false);

    cv::normalize(h1, h1, 1.0, 0.0, cv::NORM_L1);
    cv::normalize(h2, h2, 1.0, 0.0, cv::NORM_L1);

    float sum = 0.0;

    for(int i = 0; i < 256; i++){
        sum += MIN(h1.at<float>(i), h2.at<float>(i));
    }

    if (sum < 0.0){ 
        sum = 0.0;
    }
    if (sum > 1.0){ 
        sum = 1.0;
    }

    return sum;
}

int main(){
    namespace fs = std::filesystem;
    fs::path exe_path = fs::current_path();
    fs::path img_path = exe_path / "img" / "head.png";

    cv::Mat img = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    cv::Mat img2;
    cv::medianBlur(img, img2, 5);

    std::cout << "Teste: " << static_cast<float>(compute_histogram_intersection(img, img2)) << std::endl;

    return 0;
}
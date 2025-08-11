#include <opencv2/opencv.hpp>
#include <vector>
#include <iostream>
#include <filesystem>

static inline std::vector<uchar> get_neighbors(cv::Mat& img, int x, int y, uchar pad){
    std::vector<uchar> neighbors;

    for(int i = -pad; i <= pad; i++){
        for(int j = -pad; j <= pad; j++){
            neighbors.push_back(img.at<uchar>(x + i, y + j));
        }
    }

    return neighbors;
}

static inline uchar get_median(std::vector<uchar>& neighbors){
    std::nth_element(
        neighbors.begin(),
        neighbors.begin() + neighbors.size() / 2,
        neighbors.end()
    );

    return neighbors[neighbors.size() / 2];
}

cv::Mat remove_salt_and_pepper_noise(cv::Mat image){    
    cv::Mat denoised_image = image.clone();
    cv::Mat padded_img;
    
    uchar ksize = 5;
    uchar pad = ksize/2;
    cv::copyMakeBorder(image, padded_img, pad, pad, pad, pad, cv::BORDER_REFLECT_101);

    for(int i = pad; i < padded_img.rows - pad; i++){
        for(int j = pad; j < padded_img.cols - pad; j++){
            auto neighbors = get_neighbors(padded_img, i, j, pad);
            auto median = get_median(neighbors);
            denoised_image.at<uchar>(i - pad, j - pad) = median;
        }
    }

    return denoised_image;
}

int main(){
    namespace fs = std::filesystem;
    fs::path exe_path = fs::current_path();
    fs::path img_path = exe_path / "img" / "head.png";
    fs::path out_path = exe_path / "img" / "head_filtered.png";

    cv::Mat img = cv::imread(img_path, cv::IMREAD_GRAYSCALE);
    
    cv::Mat denoised_img = remove_salt_and_pepper_noise(img);

    cv::imwrite(out_path, denoised_img);
    cv::imwrite(exe_path / "head_filtered.png", denoised_img);

    return 0;
}
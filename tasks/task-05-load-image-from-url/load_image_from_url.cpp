#include <opencv2/opencv.hpp>
#include <curl/curl.h>
#include <vector>
#include <string>
#include <iostream>

static size_t writeToVector(void* ptr, size_t size, size_t nmemb, void* userdata) {
    auto* vec = static_cast<std::vector<uchar>*>(userdata);
    size_t total = size * nmemb;
    vec->insert(vec->end(), (uchar*)ptr, (uchar*)ptr + total);
    return total;
}

cv::Mat load_image_from_url(const std::string& url, int flags = cv::IMREAD_COLOR) {
    std::vector<uchar> buffer;
    CURL* curl = curl_easy_init();
    if (!curl) return cv::Mat();

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeToVector);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "curl/7.x");
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK || http_code != 200 || buffer.empty())
        return cv::Mat();

    return cv::imdecode(buffer, flags);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " <URL_da_imagem>\n";
        return 1;
    }

    cv::Mat img = load_image_from_url(argv[1]);
    if (img.empty()) {
        std::cerr << "Falha ao carregar.\n";
        return 1;
    }

    cv::imshow("Imagem", img);
    cv::waitKey(0);

    return 0;
}
#include <iostream>
#include <opencv2/opencv.hpp>

double frequency_blur_score(const cv::Mat &image, int center_size = 60) {
  if (image.empty()) {
    return 0.0;
  }

  cv::Mat gray;

  if (image.channels() == 1) {
    gray = image;
  } else if (image.channels() == 3) {
    cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
  } else if (image.channels() == 4) {
    cv::cvtColor(image, gray, cv::COLOR_BGRA2GRAY);
  } else {
    return 0.0;
  }

  cv::Mat gray32f;
  gray.convertTo(gray32f, CV_32F);

  cv::Mat planes[] = {gray32f, cv::Mat::zeros(gray32f.size(), CV_32F)};
  cv::Mat complexI;

  cv::merge(planes, 2, complexI);
  cv::dft(complexI, complexI, cv::DFT_COMPLEX_OUTPUT);

  auto fftShiftInPlace = [](cv::Mat &m) {
    const int cx = m.cols / 2;
    const int cy = m.rows / 2;

    cv::Mat q0(m, {0, 0, cx, cy});
    cv::Mat q1(m, {cx, 0, m.cols - cx, cy});
    cv::Mat q2(m, {0, cy, cx, m.rows - cy});
    cv::Mat q3(m, {cx, cy, m.cols - cx, m.rows - cy});
    cv::Mat tmp;

    q0.copyTo(tmp);
    q3.copyTo(q0);
    tmp.copyTo(q3);
    q1.copyTo(tmp);
    q2.copyTo(q1);
    tmp.copyTo(q2);
  };

  fftShiftInPlace(complexI);

  center_size = std::max(1, center_size);

  const int cx = complexI.cols / 2;
  const int cy = complexI.rows / 2;
  const int half = center_size / 2;
  const int x0 = std::max(0, cx - half);
  const int y0 = std::max(0, cy - half);
  const int w = std::min(center_size, complexI.cols - x0);
  const int h = std::min(center_size, complexI.rows - y0);

  if (w > 0 && h > 0) {
    complexI(cv::Rect(x0, y0, w, h)).setTo(cv::Scalar::all(0));
  }

  cv::split(complexI, planes);

  cv::Mat mag;
  cv::magnitude(planes[0], planes[1], mag);

  mag += 1.0f;
  cv::log(mag, mag);

  cv::Mat mask(mag.size(), CV_8UC1, cv::Scalar(255));
  if (w > 0 && h > 0)
    mask(cv::Rect(x0, y0, w, h)).setTo(0);

  const cv::Scalar m = cv::mean(mag, mask);

  return static_cast<double>(m[0]);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Uso: " << argv[0] << " <imagem> [center_size]\n";

    return 1;
  }

  const std::string path = argv[1];
  int center = (argc >= 3) ? std::stoi(argv[2]) : 60;

  cv::Mat img = cv::imread(path, cv::IMREAD_UNCHANGED);
  if (img.empty()) {
    std::cerr << "Falha ao carregar: " << path << "\n";

    return 1;
  }

  double score = frequency_blur_score(img, center);
  std::cout << "FFT blur score = " << score << "\n";

  return 0;
}

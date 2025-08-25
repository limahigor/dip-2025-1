#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat linear_combination(const Mat &i1, const Mat &i2, double a1, double a2) {
  CV_Assert(!i1.empty() && !i2.empty());
  CV_Assert(i1.size() == i2.size());
  CV_Assert(i1.type() == i2.type());

  Mat f1, f2, fOut;
  i1.convertTo(f1, CV_32F);
  i2.convertTo(f2, CV_32F);
  fOut = a1 * f1 + a2 * f2;

  Mat out;
  fOut.convertTo(out, i1.type());

  return out;
}

static Mat to_same_channels(const Mat &src, int channels) {
  if (src.channels() == channels) {
    return src;
  }

  Mat dst;

  if (channels == 3 && src.channels() == 1) {
    cv::cvtColor(src, dst, COLOR_GRAY2BGR);
  } else if (channels == 1 && src.channels() == 3) {
    cv::cvtColor(src, dst, COLOR_BGR2GRAY);
  } else {
    dst = src.clone();
  }

  return dst;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "Uso: " << argv[0] << " <img1> <img2>\n";
    cerr << "Dica: ambas serao convertidas para BGR 8-bit e img2 sera "
            "redimensionada ao tamanho de img1.\n";
    return 1;
  }

  Mat img1 = imread(argv[1], IMREAD_COLOR);
  Mat img2 = imread(argv[2], IMREAD_COLOR);
  if (img1.empty() || img2.empty()) {
    cerr << "Falha ao carregar alguma imagem.\n";
    return 1;
  }

  img2 = to_same_channels(img2, img1.channels());
  if (img2.size() != img1.size())
    resize(img2, img2, img1.size(), 0, 0, INTER_LINEAR);

  const string win = "Linear Blending";
  namedWindow(win, WINDOW_AUTOSIZE);

  int alpha_slider = 50;
  auto on_trackbar = [](int pos, void *userdata) {
    auto data = static_cast<pair<Mat, Mat> *>(userdata);
    double alpha = pos / 100.0;
    double beta = 1.0 - alpha;
    Mat blended = linear_combination(data->first, data->second, alpha, beta);
    imshow("Linear Blending (alpha)", blended);
  };

  pair<Mat, Mat> imgs{img1, img2};
  createTrackbar("alpha (%)", win, &alpha_slider, 100, on_trackbar, &imgs);

  on_trackbar(alpha_slider, &imgs);

  cout << "Controles: 's' salva (blend.png), 'q' ou 'ESC' sai.\n";
  for (;;) {
    int c = waitKey(30);
    if (c == 'q' || c == 27) {
      break;
    }
    if (c == 's') {
      double alpha = alpha_slider / 100.0;
      double beta = 1.0 - alpha;
      Mat blended = linear_combination(img1, img2, alpha, beta);
      imwrite("blend.png", blended);
      cout << "Salvo: blend.png (alpha=" << alpha << ")\n";
    }
  }

  destroyAllWindows();
  return 0;
}

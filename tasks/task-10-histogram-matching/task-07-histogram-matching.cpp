#include <iostream>
#include <opencv2/core/mat.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

vector<float> create_cdf(const Mat &channel) {
  vector<int> hist(256, 0);

  for (int y = 0; y < channel.rows; y++) {
    for (int x = 0; x < channel.cols; x++) {
      int val = channel.at<uchar>(y, x);
      hist[val]++;
    }
  }

  vector<float> cdf(256, 0.0f);
  cdf[0] = hist[0];

  for (int i = 1; i < 256; i++) {
    cdf[i] = cdf[i - 1] + hist[i];
  }

  for (int i = 0; i < 256; i++) {
    cdf[i] /= (float)(channel.rows * channel.cols);
  }

  return cdf;
}

vector<uchar> create_lut(const vector<float> &cdf_src,
                         const vector<float> &cdf_ref) {
  vector<uchar> lut(256, 0);

  for (int i = 0; i < 256; i++) {
    float val = cdf_src[i];

    int j = 0;
    float minDiff = 1.0f;
    int bestMatch = 0;

    for (j = 0; j < 256; j++) {
      float diff = fabs(val - cdf_ref[j]);

      if (diff < minDiff) {
        minDiff = diff;
        bestMatch = j;
      }
    }

    lut[i] = (uchar)bestMatch;
  }

  return lut;
}

Mat apply_lut(const Mat &ch, const vector<uchar> &lut) {
  Mat out(ch.rows, ch.cols, CV_8U);

  for (int y = 0; y < ch.rows; y++) {
    for (int x = 0; x < ch.cols; x++) {
      uchar old_val = ch.at<uchar>(y, x);
      uchar new_val = lut[old_val];
      out.at<uchar>(y, x) = new_val;
    }
  }

  return out;
}

Mat match_histograms_rgb(const Mat &source, const Mat &reference) {
  CV_Assert(source.type() == CV_8UC3 && reference.type() == CV_8UC3);

  vector<Mat> src_channels, ref_channels;
  split(source, src_channels);
  split(reference, ref_channels);

  vector<Mat> matched_channels(3);

  for (int c = 0; c < 3; c++) {
    vector<float> cdf_src = create_cdf(src_channels[c]);
    vector<float> cdf_ref = create_cdf(ref_channels[c]);

    vector<uchar> lut = create_lut(cdf_src, cdf_ref);

    matched_channels[c] = apply_lut(src_channels[c], lut);
  }

  Mat matched;
  merge(matched_channels, matched);

  return matched;
}

int main() {
  Mat source = imread("source.jpg");
  Mat reference = imread("reference.jpg");

  if (source.empty() || reference.empty()) {
    cout << "Erro ao carregar imagens!" << endl;
    return -1;
  }

  Mat matched = match_histograms_rgb(source, reference);
  imwrite("matched.jpg", matched);

  return 0;
}

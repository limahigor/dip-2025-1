#include <cmath>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

static void preparePair(const Mat &in1, const Mat &in2, Mat &A, Mat &B) {
  CV_Assert(!in1.empty() && !in2.empty());
  CV_Assert(in1.size() == in2.size());
  CV_Assert(in1.channels() == 1 && in2.channels() == 1);

  Mat a64, b64;
  in1.convertTo(a64, CV_64F);
  in2.convertTo(b64, CV_64F);

  A = a64.isContinuous() ? a64 : a64.clone();
  B = b64.isContinuous() ? b64 : b64.clone();
}

struct MomentsAB {
  double muA, muB;
  double varA, varB;
  double covAB;
  double mse;
};

static MomentsAB computeMoments(const Mat &A, const Mat &B) {
  CV_Assert(A.type() == CV_64F && B.type() == CV_64F);
  CV_Assert(A.size() == B.size() && A.channels() == 1 && B.channels() == 1);

  const int N = A.rows * A.cols;
  const double *pa = reinterpret_cast<const double *>(A.ptr());
  const double *pb = reinterpret_cast<const double *>(B.ptr());

  long double sumA = 0, sumB = 0, sumA2 = 0, sumB2 = 0, sumAB = 0;
  for (int i = 0; i < N; ++i) {
    double a = pa[i], b = pb[i];
    sumA += a;
    sumB += b;
    sumA2 += a * a;
    sumB2 += b * b;
    sumAB += a * b;
  }
  double muA = static_cast<double>(sumA / N);
  double muB = static_cast<double>(sumB / N);

  // E[x^2] - (E[x])^2
  double Ex2A = static_cast<double>(sumA2 / N);
  double Ex2B = static_cast<double>(sumB2 / N);
  double varA = Ex2A - muA * muA;
  double varB = Ex2B - muB * muB;

  // E[xy] - E[x]E[y]
  double Exy = static_cast<double>(sumAB / N);
  double covAB = Exy - muA * muB;

  // MSE = E[(A-B)^2] = E[A^2] + E[B^2] - 2E[AB]
  double mse = Ex2A + Ex2B - 2.0 * Exy;

  return {muA, muB, varA, varB, covAB, mse};
}

static double MSE(const Mat &A, const Mat &B) {
  return computeMoments(A, B).mse;
}

static double PSNR_from_MSE(double mse, double L = 1.0) {
  if (mse <= 0.0)
    return numeric_limits<double>::infinity();
  return 10.0 * std::log10((L * L) / mse);
}
static double PSNR(const Mat &A, const Mat &B, double L = 1.0) {
  return PSNR_from_MSE(MSE(A, B), L);
}

static double SSIM_simplified(const Mat &A, const Mat &B, double C1 = 1e-8,
                              double C2 = 1e-8) {
  auto m = computeMoments(A, B);
  double num = (2.0 * m.muA * m.muB + C1) * (2.0 * m.covAB + C2);
  double denom = (m.muA * m.muA + m.muB * m.muB + C1) * (m.varA + m.varB + C2);
  
  if (denom == 0.0) {
    return (num == 0.0) ? 1.0 : 0.0;

  }
  return num / denom;
}

static double NPCC(const Mat &A, const Mat &B) {
  auto m = computeMoments(A, B);
  double denom =
      std::sqrt(std::max(0.0, m.varA)) * std::sqrt(std::max(0.0, m.varB));
  if (denom == 0.0) {
    return std::numeric_limits<double>::quiet_NaN();
  }

  return m.covAB / denom;
}

int main(int argc, char **argv) {
  if (argc < 3) {
    cerr << "Uso: " << argv[0] << " <img1> <img2>\n"
         << "Ambas serao lidas em escala de cinza e normalizadas em [0,1].\n";
    return 1;
  }

  Mat g1 = imread(argv[1], IMREAD_GRAYSCALE);
  Mat g2 = imread(argv[2], IMREAD_GRAYSCALE);

  if (g1.empty() || g2.empty()) {
    cerr << "Falha ao carregar alguma imagem.\n";
    return 1;
  }

  if (g1.size() != g2.size()) {
    cerr << "As imagens devem ter o mesmo tamanho.\n";
    return 1;
  }

  Mat A, B;
  g1.convertTo(A, CV_64F, 1.0 / 255.0);
  g2.convertTo(B, CV_64F, 1.0 / 255.0);
  preparePair(A, B, A, B);

  double mse = MSE(A, B);
  double psnr = PSNR_from_MSE(mse, /*L=*/1.0);
  double ssim = SSIM_simplified(A, B, 1e-8, 1e-8);
  double npcc = NPCC(A, B);

  cout.setf(std::ios::fixed);
  cout.precision(6);

  cout << "MSE  : " << mse << "\n";

  if (std::isinf(psnr)) {
    cout << "PSNR : inf (imagens identicas)\n";
  } else {
    cout << "PSNR : " << psnr << " dB\n";
  }

  cout << "SSIM : " << ssim << "\n";

  if (std::isnan(npcc)) {
    cout << "NPCC : NaN (variancia zero)\n";
  } else {
    cout << "NPCC : " << npcc << "\n";
  }
  return 0;
}

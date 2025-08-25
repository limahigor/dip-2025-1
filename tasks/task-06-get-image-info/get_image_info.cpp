#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <sstream>

struct ChannelStats {
    double min = 0, max = 0, mean = 0, std_dev = 0, median = 0;
    std::vector<uint64_t> histogram;
};

struct ImageInfo {
    int width = 0, height = 0;
    std::string dtype;
    int depth = 0;
    size_t nbytes = 0;
    std::map<std::string, ChannelStats> statistics;
};

static std::string dtype_from_mat(int cv_depth) {
    switch (cv_depth) {
        case CV_8U:  return "uint8";
        case CV_8S:  return "int8";
        case CV_16U: return "uint16";
        case CV_16S: return "int16";
        case CV_32S: return "int32";
        case CV_32F: return "float32";
        case CV_64F: return "float64";
        default:     return "unknown";
    }
}


static double median_from_hist_8u(const cv::Mat& hist) {
    double total = cv::sum(hist)[0], acc = 0.0;
    for (int i = 0; i < hist.rows; ++i) {
        acc += hist.at<float>(i);
        if (acc >= total * 0.5) return i;
    }
    return 0.0;
}


static double median_generic(const cv::Mat& ch) {
    CV_Assert(ch.channels() == 1);
    cv::Mat flat = ch.reshape(1, (int)ch.total());
    cv::Mat flat64; flat.convertTo(flat64, CV_64F);
    auto* beg = reinterpret_cast<double*>(flat64.data);
    auto* end = beg + flat64.total();
    size_t n = flat64.total(), m = n/2;
    std::nth_element(beg, beg + m, end);
    if (n % 2) return beg[m];
    auto max_it = std::max_element(beg, beg + m);
    return (beg[m] + *max_it) / 2.0;
}


static cv::Mat calc_hist_8u(const cv::Mat& ch) {
    CV_Assert(ch.type() == CV_8UC1);
    int histSize = 256; float range[] = {0.f, 256.f}; const float* ranges[] = {range};
    int channels[] = {0}; cv::Mat hist;
    cv::calcHist(&ch, 1, channels, cv::Mat(), hist, 1, &histSize, ranges, true, false);
    return hist;
}


static bool is_pseudo_gray_bgr(const cv::Mat& bgr, int tol=3, double frac=0.02) {
    std::vector<cv::Mat> ch; cv::split(bgr, ch);
    cv::Mat dBG, dBR, dGR;
    cv::absdiff(ch[0], ch[1], dBG);
    cv::absdiff(ch[0], ch[2], dBR);
    cv::absdiff(ch[1], ch[2], dGR);
    cv::Mat colored = (dBG > tol) | (dBR > tol) | (dGR > tol);
    double ratio = cv::countNonZero(colored) / (double)bgr.total();
    return ratio <= frac;
}


ImageInfo get_image_info(const cv::Mat& image_in) {
    if (image_in.empty()) throw std::runtime_error("Input must be a valid cv::Mat.");

    cv::Mat img = image_in;
    if (img.channels() == 4) cv::cvtColor(img, img, cv::COLOR_BGRA2BGR);

    ImageInfo info;
    info.width  = img.cols;
    info.height = img.rows;
    info.depth  = img.channels();
    info.dtype  = dtype_from_mat(img.depth());   // tipo do CANAL
    info.nbytes = img.total() * img.elemSize();

    bool treat_as_gray = (info.depth == 1);
    if (!treat_as_gray && info.depth >= 3 && img.depth() == CV_8U && is_pseudo_gray_bgr(img))
        treat_as_gray = true;

    if (treat_as_gray) {
        cv::Mat gray = (img.channels()==1)? img : [&]{ cv::Mat g; cv::cvtColor(img, g, cv::COLOR_BGR2GRAY); return g; }();

        ChannelStats st;
        double mn, mx; cv::Point p1, p2; cv::minMaxLoc(gray, &mn, &mx, &p1, &p2);
        cv::Scalar m, s; cv::meanStdDev(gray, m, s);
        st.min = mn; st.max = mx; st.mean = m[0]; st.std_dev = s[0];

        if (gray.type()==CV_8UC1) {
            cv::Mat hist = calc_hist_8u(gray);
            st.median = median_from_hist_8u(hist);
            st.histogram.resize(256);
            for (int i=0;i<256;++i) st.histogram[i] = (uint64_t)std::llround(hist.at<float>(i));
        } else {
            st.median = median_generic(gray);
        }
        info.depth = 1;
        info.statistics["gray"] = std::move(st);
    } else {
        std::vector<cv::Mat> ch; cv::split(img, ch);
        for (int i=0;i<(int)ch.size();++i) {
            ChannelStats st;
            double mn, mx; cv::Point p1, p2; cv::minMaxLoc(ch[i], &mn, &mx, &p1, &p2);
            cv::Scalar m, s; cv::meanStdDev(ch[i], m, s);
            st.min = mn; st.max = mx; st.mean = m[0]; st.std_dev = s[0];
            if (img.depth()==CV_8U) {
                cv::Mat hist = calc_hist_8u(ch[i]);
                st.median = median_from_hist_8u(hist);
                st.histogram.resize(256);
                for (int b=0;b<256;++b) st.histogram[b] = (uint64_t)std::llround(hist.at<float>(b));
            } else st.median = median_generic(ch[i]);
            info.statistics["channel_" + std::to_string(i)] = std::move(st);
        }
    }
    return info;
}


static std::string fmt_count(double v) {
    std::ostringstream os; os.setf(std::ios::fixed); os.precision(0);
    if (v >= 1e6) { os.precision(1); os << v/1e6 << "M"; }
    else if (v >= 1e3){ os.precision(1); os << v/1e3 << "k"; }
    else os << v;
    return os.str();
}


static double nice_step(double maxv, int target=5) {
    if (maxv <= 0) return 1.0;
    double raw = maxv / target;
    double mag = std::pow(10.0, std::floor(std::log10(raw)));
    double n = raw / mag;
    double step;
    if (n < 1.5) step = 1.0;
    else if (n < 3.0) step = 2.0;
    else if (n < 7.0) step = 5.0;
    else step = 10.0;
    return step * mag;
}


static cv::Mat render_hist_gray(const std::vector<uint64_t>& hist,
                                                int W=320, int H=240) {
    cv::Mat canvas(H, W, CV_8UC3, cv::Scalar(20,20,20));
    if (hist.size()!=256) return canvas;

    const int L=50, R=10, T=10, B=35;
    const int PW = W - L - R, PH = H - T - B;
    const cv::Point origin(L, H - B);

    cv::rectangle(canvas, cv::Rect(L,T,PW,PH), cv::Scalar(40,40,40), 1);
    cv::line(canvas, origin, {L+PW, H-B}, cv::Scalar(100,100,100), 1);
    cv::line(canvas, origin, {L,    T   }, cv::Scalar(100,100,100), 1);

    uint64_t maxv = *std::max_element(hist.begin(), hist.end());
    uint64_t minv = *std::min_element(hist.begin(), hist.end());
    if (maxv == 0) return canvas;
    double step = nice_step(maxv, 5);
    double ymax = std::ceil(maxv/step) * step;

    for (double v=0; v<=ymax+1e-9; v+=step) {
        int y = origin.y - (int)std::round((v/ymax) * PH);
        cv::line(canvas, {L-5,y}, {L,y}, cv::Scalar(180,180,180), 1);
        cv::putText(canvas, fmt_count(v), {5,y+4},
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1, cv::LINE_AA);
        if (v>0 && v<ymax) cv::line(canvas, {L,y}, {L+PW,y}, cv::Scalar(60,60,60), 1, cv::LINE_AA);
    }

    auto x_from_bin = [&](int b){ return L + (int)std::round((b/255.0) * PW); };
    for (int b : {0,64,128,192,255}) {
        int x = x_from_bin(b);
        cv::line(canvas, {x,origin.y},{x,origin.y+5}, cv::Scalar(180,180,180), 1);
        cv::putText(canvas, std::to_string(b), {x-12, H-12},
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1, cv::LINE_AA);
    }

    if (maxv == minv) {
        int y = origin.y - (int)std::round((hist[0]/ymax) * PH);
        cv::line(canvas, {L, y}, {L+PW, y}, cv::Scalar(255,255,255), 2, cv::LINE_AA);
    } else {
        auto y_from = [&](int i){
            double h = (hist[i]/ymax) * PH;
            return origin.y - (int)std::round(h);
        };
        for (int i=1;i<256;++i) {
            int x0 = x_from_bin(i-1), x1 = x_from_bin(i);
            cv::line(canvas, {x0, y_from(i-1)}, {x1, y_from(i)}, cv::Scalar(255,255,255), 2, cv::LINE_AA);
        }
    }

    return canvas;
}


static cv::Mat render_hist_rgb(const ChannelStats& Bc,
                                               const ChannelStats& Gc,
                                               const ChannelStats& Rc,
                                               int W=320, int H=240) {
    cv::Mat canvas(H, W, CV_8UC3, cv::Scalar(20,20,20));
    if (Bc.histogram.size()!=256 || Gc.histogram.size()!=256 || Rc.histogram.size()!=256)
        return canvas;

    const int L=50, R=10, T=10, B=35;
    const int PW = W - L - R, PH = H - T - B;
    const cv::Point origin(L, H - B);

    cv::rectangle(canvas, cv::Rect(L,T,PW,PH), cv::Scalar(40,40,40), 1);
    cv::line(canvas, origin, {L+PW, H-B}, cv::Scalar(100,100,100), 1);
    cv::line(canvas, origin, {L,    T   }, cv::Scalar(100,100,100), 1);

    uint64_t maxb = *std::max_element(Bc.histogram.begin(), Bc.histogram.end());
    uint64_t maxg = *std::max_element(Gc.histogram.begin(), Gc.histogram.end());
    uint64_t maxr = *std::max_element(Rc.histogram.begin(), Rc.histogram.end());
    uint64_t maxv = std::max(maxb, std::max(maxg, maxr));
    if (maxv==0) return canvas;
    double step = nice_step(maxv, 5);
    double ymax = std::ceil(maxv/step) * step;

    for (double v=0; v<=ymax+1e-9; v+=step) {
        int y = origin.y - (int)std::round((v/ymax) * PH);
        cv::line(canvas, {L-5,y}, {L,y}, cv::Scalar(180,180,180), 1);
        cv::putText(canvas, fmt_count(v), {5,y+4},
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1, cv::LINE_AA);
        if (v>0 && v<ymax) cv::line(canvas, {L,y}, {L+PW,y}, cv::Scalar(60,60,60), 1, cv::LINE_AA);
    }
    auto x_from_bin = [&](int b){ return L + (int)std::round((b/255.0) * PW); };
    for (int b : {0,64,128,192,255}) {
        int x = x_from_bin(b);
        cv::line(canvas, {x,origin.y},{x,origin.y+5}, cv::Scalar(180,180,180), 1);
        cv::putText(canvas, std::to_string(b), {x-12, H-12},
                    cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(200,200,200), 1, cv::LINE_AA);
    }

    auto draw = [&](const std::vector<uint64_t>& hist, cv::Scalar color){
        auto y_from = [&](int i){
            double h = (hist[i]/ymax) * PH;
            return origin.y - (int)std::round(h);
        };
        for (int i=1;i<256;++i) {
            int x0 = x_from_bin(i-1), x1 = x_from_bin(i);
            cv::line(canvas, {x0, y_from(i-1)}, {x1, y_from(i)}, color, 2, cv::LINE_AA);
        }
    };
    draw(Bc.histogram, {255,0,0});
    draw(Gc.histogram, {0,255,0});
    draw(Rc.histogram, {0,0,255});

    cv::putText(canvas, "B", {L+10, T+18}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {255,0,0}, 2, cv::LINE_AA);
    cv::putText(canvas, "G", {L+35, T+18}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {0,255,0}, 2, cv::LINE_AA);
    cv::putText(canvas, "R", {L+60, T+18}, cv::FONT_HERSHEY_SIMPLEX, 0.6, {0,0,255}, 2, cv::LINE_AA);

    return canvas;
}


static void paste_resized(const cv::Mat& src, cv::Mat& dst, const cv::Rect& roi) {
    CV_Assert(roi.x >= 0 && roi.y >= 0 &&
              roi.x + roi.width  <= dst.cols &&
              roi.y + roi.height <= dst.rows);

    cv::Mat tmp;
    cv::resize(src, tmp, roi.size(), 0, 0, cv::INTER_AREA);

    if (tmp.channels() != dst.channels()) {
        if (tmp.channels() == 1 && dst.channels() == 3) {
            cv::cvtColor(tmp, tmp, cv::COLOR_GRAY2BGR);
        } else if (tmp.channels() == 3 && dst.channels() == 1) {
            cv::cvtColor(tmp, tmp, cv::COLOR_BGR2GRAY);
        } else {
            CV_Assert(false && "Conversao de canais nao suportada nesta funcao");
        }
    }

    tmp.copyTo(dst(roi));
}


int main() {
    cv::Mat gray(256, 256, CV_8UC1);
    for (int y=0;y<gray.rows;++y)
        for (int x=0;x<gray.cols;++x)
            gray.at<uchar>(y,x) = (uchar)x;

    cv::Mat small(8,8,CV_8UC1);
    for (int y=0;y<8;++y) for (int x=0;x<8;++x) small.at<uchar>(y,x) = ((x+y)&1)?255:0;
    cv::Mat checker; cv::resize(small, checker, {256,256}, 0,0, cv::INTER_NEAREST);

    cv::Mat rgb(100,100,CV_8UC3);
    cv::randn(rgb, cv::Scalar(128,128,128), cv::Scalar(20,20,20));

    ImageInfo gray_info = get_image_info(gray);
    ImageInfo chk_info  = get_image_info(checker);
    ImageInfo rgb_info  = get_image_info(rgb);

    cv::Mat hist_gray = render_hist_gray(gray_info.statistics.at("gray").histogram, 320, 240);
    cv::Mat hist_chk  = render_hist_gray(chk_info.statistics.at("gray").histogram, 320, 240);
    const auto& B = rgb_info.statistics.at("channel_0");
    const auto& G = rgb_info.statistics.at("channel_1");
    const auto& R = rgb_info.statistics.at("channel_2");
    cv::Mat hist_rgb = render_hist_rgb(B, G, R, 320, 240);

    const int CELL_W = 320, CELL_H_TOP = 240, CELL_H_BOT = 240;
    const int M = 20; // margem
    int panelW = 3*CELL_W + 4*M;
    int panelH = CELL_H_TOP + CELL_H_BOT + 3*M;

    cv::Mat panel(panelH, panelW, CV_8UC3, cv::Scalar(10,10,10));

    paste_resized(gray,    panel, {M + 0*(CELL_W+M), M, CELL_W, CELL_H_TOP});
    paste_resized(checker, panel, {M + 1*(CELL_W+M), M, CELL_W, CELL_H_TOP});
    cv::Mat rgb_vis; cv::resize(rgb, rgb_vis, {CELL_W, CELL_H_TOP}, 0,0, cv::INTER_LINEAR);
    paste_resized(rgb_vis, panel, {M + 2*(CELL_W+M), M, CELL_W, CELL_H_TOP});

    paste_resized(hist_gray, panel, {M + 0*(CELL_W+M), 2*M + CELL_H_TOP, CELL_W, CELL_H_BOT});
    paste_resized(hist_chk,  panel, {M + 1*(CELL_W+M), 2*M + CELL_H_TOP, CELL_W, CELL_H_BOT});
    paste_resized(hist_rgb,  panel, {M + 2*(CELL_W+M), 2*M + CELL_H_TOP, CELL_W, CELL_H_BOT});

    cv::putText(panel, "Grayscale Gradient", {M+10, M+20},
                cv::FONT_HERSHEY_SIMPLEX, 0.6, {240,240,240}, 2, cv::LINE_AA);
    cv::putText(panel, "Checkerboard", {M+CELL_W+10, M+20},
                cv::FONT_HERSHEY_SIMPLEX, 0.6, {240,240,240}, 2, cv::LINE_AA);
    cv::putText(panel, "Random RGB", {M+2*(CELL_W+M)+10, M+20},
                cv::FONT_HERSHEY_SIMPLEX, 0.6, {240,240,240}, 2, cv::LINE_AA);

    cv::imshow("Histograma", panel);
    cv::imwrite("panel_histograms.png", panel);


    auto print_info = [](const std::string& title, const ImageInfo& I){
        std::cout << "\n" << title << "\n"
                  << "{ width: " << I.width
                  << ", height: " << I.height
                  << ", dtype: \"" << I.dtype << "\""
                  << ", depth: " << I.depth
                  << ", nbytes: " << I.nbytes << ", statistics: {\n";
        for (auto& kv : I.statistics) {
            const auto& k = kv.first; const auto& s = kv.second;
            std::cout << "  " << k << ": { min:" << s.min << ", max:" << s.max
                      << ", mean:" << s.mean << ", std_dev:" << s.std_dev
                      << ", median:" << s.median << ", hist_len:" << s.histogram.size() << " }\n";
        }
        std::cout << "} }\n";
    };

    print_info("Grayscale Image Info:", gray_info);
    print_info("Checkerboard Image Info:", chk_info);
    print_info("Random RGB Image Info:", rgb_info);

    cv::waitKey(0);
    return 0;
}

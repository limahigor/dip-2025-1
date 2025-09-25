#include <bits/stdc++.h>
using namespace std;

using Image = vector<vector<int>>;

Image translate(const Image &img, int dx, int dy) {
  int h = img.size();
  int w = img[0].size();

  Image out(h, vector<int>(w, 0));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      int nx = x + dx;
      int ny = y + dy;
      if (ny >= 0 && ny < h && nx >= 0 && nx < w) {
        out[ny][nx] = img[y][x];
      }
    }
  }

  return out;
}

Image rotate(const Image &img) {
  int h = img.size();
  int w = img[0].size();

  Image out(w, vector<int>(h, 0));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      out[x][h - 1 - y] = img[y][x];
    }
  }

  return out;
}

Image stretch(const Image &img, double factor) {
  int h = img.size();
  int w = img[0].size();
  int newW = int(w * factor);

  Image out(h, vector<int>(newW, 0));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < newW; x++) {
      int srcX = int(x / factor);

      if (srcX >= 0 && srcX < w) {
        out[y][x] = img[y][srcX];
      }
    }
  }

  return out;
}

Image mirror(const Image &img) {
  int h = img.size();
  int w = img[0].size();

  Image out(h, vector<int>(w, 0));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      out[y][w - 1 - x] = img[y][x];
    }
  }

  return out;
}

Image distortion(const Image &img, double k = 0.05) {
  int h = img.size();
  int w = img[0].size();

  Image out(h, vector<int>(w, 0));

  double cx = w / 2.0;
  double cy = h / 2.0;

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      double dx = (x - cx) / cx;
      double dy = (y - cy) / cy;
      double r2 = dx * dx + dy * dy;

      double factor = 1 + k * r2;

      int srcX = int(cx + dx * factor * cx);
      int srcY = int(cy + dy * factor * cy);

      if (srcX >= 0 && srcX < w && srcY >= 0 && srcY < h) {
        out[y][x] = img[srcY][srcX];
      }
    }
  }

  return out;
}

map<string, Image> apply_geometric_transformations(const Image &img) {
  map<string, Image> result;

  result["translated"] = translate(img, 10, 10);
  result["rotated"] = rotate(img);
  result["stretched"] = stretch(img, 1.5);
  result["mirrored"] = mirror(img);
  result["distorted"] = distortion(img);

  return result;
}

void save_pgm(const Image &img, const string &filename) {
  int h = img.size();
  int w = img[0].size();

  ofstream out(filename);

  out << "P2\n" << w << " " << h << "\n255\n";

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      out << img[y][x] << " ";
    }
    out << "\n";
  }

  out.close();
}

Image generate_test_image(int w, int h) {
  Image img(h, vector<int>(w, 0));

  for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      if (x < w / 3) {
        img[y][x] = 50;
      } else if (x < 2 * w / 3) {
        img[y][x] = 150;
      } else {
        img[y][x] = 250;
      }
    }
  }

  return img;
}

int main() {
  Image img = generate_test_image(200, 200);

  auto res = apply_geometric_transformations(img);

  save_pgm(img, "original.pgm");
  save_pgm(res["translated"], "translated.pgm");
  save_pgm(res["rotated"], "rotated.pgm");
  save_pgm(res["stretched"], "stretched.pgm");
  save_pgm(res["mirrored"], "mirrored.pgm");
  save_pgm(res["distorted"], "distorted.pgm");

  return 0;
}

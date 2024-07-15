
#include "solution.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <ios>
#include <stdint.h>

// ******************************************
// ONLY THE FOLLOWING FUNCTION IS BENCHMARKED
// Compute the histogram of image pixels
std::array<uint32_t, 256> computeHistogram(const GrayscaleImage &image) {
  std::array<uint32_t, 256> hist;
  hist.fill(0);

#define UPDATE_HIST(index) hist_temp[256 * (index) + image.data[i + (index)]]++;

// Helper macros to create a chain of updates
#define UPDATE_1(i) UPDATE_HIST(i)
#define UPDATE_2(i) UPDATE_1(i) UPDATE_1(i + 1)
#define UPDATE_4(i) UPDATE_2(i) UPDATE_2(i + 2)
#define UPDATE_8(i) UPDATE_4(i) UPDATE_4(i + 4)
#define UPDATE_16(i) UPDATE_8(i) UPDATE_8(i + 8)
#define UPDATE_32(i) UPDATE_16(i) UPDATE_16(i + 16)

  static constexpr int kN = 5;
  uint32_t hist_temp[256 * (1 << kN)]{0};
  for (int i = 0; i < image.width * image.height; i += (1 << kN)) {
    UPDATE_32(0)
  }
  for (int i = 0; i < 256; i++) {
    for (int j = 0; j < 1 << kN; j++) {
      hist[i] += hist_temp[j * 256 + i];
    }
  }
  return hist;
}
// ******************************************

// Calculate Otsu's Threshold
int calcOtsuThreshold(const std::array<uint32_t, 256> &hist, int totalPixels) {
  // normalize histogram
  std::array<double, 256> normHist;
  for (int i = 0; i < 256; ++i)
    normHist[i] = (double)hist[i] / totalPixels;

  double maxVariance = 0;
  int optimalThreshold = 0;

  // Find the optimal threshold
  for (int t = 0; t < 256; ++t) {
    double weight1 = 0, weight2 = 0, mean1 = 0, mean2 = 0;

    for (int i = 0; i <= t; ++i) {
      weight1 += normHist[i];
      mean1 += i * normHist[i];
    }

    for (int i = t + 1; i < 256; ++i) {
      weight2 += normHist[i];
      mean2 += i * normHist[i];
    }

    if (weight1 == 0 || weight2 == 0)
      continue;

    mean1 /= weight1;
    mean2 /= weight2;

    double variance = weight1 * weight2 * std::pow(mean1 - mean2, 2);

    if (variance > maxVariance) {
      maxVariance = variance;
      optimalThreshold = t;
    }
  }

  return optimalThreshold;
}

// Function to apply the threshold to create a binary image
void applyOtsuThreshold(GrayscaleImage &image) {
  // Compute the histogram
  std::array<uint32_t, 256> hist = computeHistogram(image);
  auto totalPixels = image.height * image.width;
  int threshold = calcOtsuThreshold(hist, totalPixels);
  // Apply Otsu's thresholding
  for (int i = 0; i < totalPixels; ++i)
    image.data[i] = (image.data[i] >= threshold) ? 255 : 0;
}

// Loads GrayscaleImage image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html
bool GrayscaleImage::load(const std::string &filename, const int maxSize) {
  data.reset();

  std::ifstream input(filename.data(),
                      std::ios_base::in | std::ios_base::binary);
  if (input.is_open()) {
    std::string line;
    input >> line;
    if (line == "P5") {
      int amplitude;
      input >> width >> height >> amplitude;

      char c;
      input.unsetf(std::ios_base::skipws);
      input >> c;
      if (c == '\r')
        input >> c;

      if ((width > 0) && (width <= maxSize) && (height > 0) &&
          (height <= maxSize) && (amplitude >= 0) && (amplitude <= 255) &&
          (c == '\n')) {
        size = static_cast<size_t>(width) * static_cast<size_t>(height);
        data.reset(new uint8_t[size]);
        if (data) {
          input.read(reinterpret_cast<char *>(data.get()), size);
          if (input.fail()) {
            data.reset();
          }
        }
      }
    }

    input.close();
  }

  return !!data;
}

// Saves GrayscaleImage image. Format is
// https://people.sc.fsu.edu/~jburkardt/data/pgmb/pgmb.html
void GrayscaleImage::save(const std::string &filename) {
  std::ofstream output(filename.data(),
                       std::ios_base::out | std::ios_base::binary);
  if (output.is_open()) {
    output << "P5" << std::endl
           << width << ' ' << height << std::endl
           << "255" << std::endl;
    if (data) {
      output.write(reinterpret_cast<const char *>(data.get()), size);
    }
    output.close();
  }
}

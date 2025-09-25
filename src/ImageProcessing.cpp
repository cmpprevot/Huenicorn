#include <Huenicorn/ImageProcessing.hpp>


namespace Huenicorn
{
  namespace ImageProcessing
  {
    void rescale(cv::Mat& image, int targetWidth, Interpolation::Type interpolation)
    {
      int sourceHeight = image.rows;
      int sourceWidth = image.cols;

      if(sourceWidth < targetWidth){
        return;
      }

      float scaleRatio = static_cast<float>(targetWidth) / sourceWidth;

      int targetHeight = sourceHeight * scaleRatio;

      cv::InterpolationFlags interpolationFlag = cv::InterpolationFlags::INTER_AREA;

      switch(interpolation){
        case Interpolation::Type::Nearest:
          interpolationFlag = cv::InterpolationFlags::INTER_NEAREST;
          break;

        case Interpolation::Type::Cubic:
          interpolationFlag = cv::InterpolationFlags::INTER_CUBIC;
          break;

        case Interpolation::Type::Area:
          interpolationFlag = cv::InterpolationFlags::INTER_AREA;
          break;
      }

      cv::resize(image, image, cv::Size(targetWidth, targetHeight), 0, 0, interpolationFlag);
    }


    cv::Mat getSubImage(const cv::Mat& sourceImage, const glm::ivec2& a, const glm::ivec2& b)
    {
      cv::Range cols(std::max(0, a.x), std::min(b.x, sourceImage.cols));
      cv::Range rows(std::max(0, a.y), std::min(b.y, sourceImage.rows));

      return sourceImage(rows, cols);
    }


    Color getDominantColor(cv::Mat& image)
    {
      if(image.cols < 1 || image.rows < 1){
        return {Color(0, 0, 0)};
      }

      return Algorithms::mean(image);
    }


    namespace Algorithms
    {
      Color kMeans(const cv::Mat& image)
      {
        unsigned k = 1;
        Colors dominantColors;
        dominantColors.reserve(k);
        cv::Mat data = image.reshape(1, image.total());
        data.convertTo(data, CV_32F);

        cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1.0);

        std::vector<int> labels;
        cv::Mat centers;
        int flags = cv::KmeansFlags::KMEANS_PP_CENTERS;
        //int flags = cv::KmeansFlags::KMEANS_RANDOM_CENTERS;
        cv::kmeans(data, k, labels, criteria, 10, flags, centers);

        centers.convertTo(centers, CV_8U);

        for(int i = 0; i < centers.rows; i++){
          const uint8_t* Mi = centers.ptr<uint8_t>(i);
          dominantColors.emplace_back(Mi[2], Mi[1], Mi[0]);
        }

        return dominantColors.front();
      }


      Color mean(const cv::Mat& image)
      {
        cv::Mat data = image.reshape(3, image.total());
        auto mean = cv::mean(data);

        return Color{
          static_cast<uint8_t>(mean[2]),
          static_cast<uint8_t>(mean[1]),
          static_cast<uint8_t>(mean[0])
        };
      }
    }
  }
}

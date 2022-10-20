#include <FreenSync/ImageProcessor.hpp>

using namespace std;

int main()
{
  ImageProcessor ip;

  ImageData id;
  ImageProcessor::getScreenCapture(id);
  cv::Mat img = cv::Mat(id.height, id.width, id.bitsPerPixel > 24 ? CV_8UC4 : CV_8UC3, id.pixels.data());

  cv::imshow("Display window", img);
  cv::waitKey(0);
  ImageProcessor::rescale(img, 100);
  cv::imshow("Display window", img);
  cv::waitKey(0);

  Colors colors = ip.getDominantColors(img, 2);

  for(const Color& color : colors){
    cout << color.toStr() << endl;
    cv::Mat mat(480, 640, CV_8UC3, color.toScalar());
    cv::imshow("Display window", mat);
    cv::waitKey(0);
  }

  return 0;
}

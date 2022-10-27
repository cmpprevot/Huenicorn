#include <FreenSync/ImageProcessor.hpp>
#include <FreenSync/BridgeData.hpp>

#include <thread>

using namespace std;

int main()
{
  BridgeData bd;

  auto& lights = bd.lights();

  if(lights.size() > 0){
    auto& light = lights.back();

    for(int i = 0; i < 100; i++){
      light->setState(!light->state());
      this_thread::sleep_for(0.5s);
    }
  }

  /*
  for(const auto& light : bd.lights()){
    cout << light.dump(2) << endl;
  }
  */


  /*
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
  */

  return 0;
}

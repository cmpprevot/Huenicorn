#include <FreenSync/ImageProcessor.hpp>
#include <FreenSync/BridgeData.hpp>

#include <thread>

#include <glm/gtx/string_cast.hpp>

using namespace std;

int main()
{
  BridgeData bd;
  //*/
  auto& lights = bd.lights();

  if(lights.size() > 0){
    auto& light = lights.back();

    int max = 255;
    // Loop on primary colors to check color conversion accuracy
    for(int i = 0; i < 32; i++){
      light->setColor(Color(
        (i & 1) * max,
        (i & 2) * max,
        (i & 4) * max)
      );
      this_thread::sleep_for(1s);
    }
  }
  /*/

  SharedLight light = make_shared<Light>(&bd, "test", nlohmann::json::object());
  light->setColor({0, 127, 255});
  //*/

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

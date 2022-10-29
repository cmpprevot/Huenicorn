#include <FreenSync/FreenSync.hpp>

#include <thread>

#include <glm/gtx/string_cast.hpp>

using namespace std;

void testPrimaryColors()
{
  BridgeData bd;
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
}


void startFreenSync()
{
  FreenSync fs;

  fs.start(20.f);

  cout << "Hit enter to stop" << endl;
  cin.get();
  fs.stop();
}


int main()
{
  startFreenSync();

  return 0;
}

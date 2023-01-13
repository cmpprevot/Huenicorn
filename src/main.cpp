#include <thread>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/RestServer.hpp>


using namespace std;


void startHuenicorn()
{
  Huenicorn::HuenicornCore core;
  core.start();

  cout << "Hit enter to stop" << endl;
  cin.get();

  core.stop();
}


int main()
{
  startHuenicorn();

  return 0;
}

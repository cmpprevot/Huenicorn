#include <thread>

#include <FreenSync/FreenSync.hpp>
#include <FreenSync/RestServer.hpp>


using namespace std;


void startFreenSync()
{
  FreenSync fs;
  fs.start();

  cout << "Hit enter to stop" << endl;
  cin.get();

  fs.stop();
}


int main()
{
  startFreenSync();

  return 0;
}

#include <thread>

#include <FreenSync/FreenSyncCore.hpp>
#include <FreenSync/RestServer.hpp>


using namespace std;


void startFreenSync()
{
  FreenSync::FreenSyncCore core;
  core.start();

  cout << "Hit enter to stop" << endl;
  cin.get();

  core.stop();
}


int main()
{
  startFreenSync();

  return 0;
}

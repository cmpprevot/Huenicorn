#include <thread>

#include <FreenSync/FreenSync.hpp>
#include <FreenSync/RestServer.hpp>


using namespace std;


void startFreenSync()
{
  SharedFreenSync fs = make_shared<FreenSync>();

  fs->start();

  RestServer rs(fs, 8080);
  rs.start();

  cout << "Hit enter to stop" << endl;
  cin.get();
  fs->stop();
}


int main()
{
  startFreenSync();

  return 0;
}

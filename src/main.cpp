#include <thread>

#include <Huenicorn/HuenicornCore.hpp>
#include <Huenicorn/RestServer.hpp>
#include <csignal>

using namespace std;


class Application
{
public:
  void start()
  {
    m_core = make_unique<Huenicorn::HuenicornCore>();
    m_applicationThread.emplace([&](){
      m_core->start();
    });

    m_applicationThread.value().join();
    m_applicationThread.reset();
    m_core.reset();
  }


  void stop()
  {
    if(!m_core){
      return;
    }

    m_core->stop();
  }

private:
  unique_ptr<Huenicorn::HuenicornCore> m_core;
  std::optional<thread> m_applicationThread;
};


Application app;


void signalHandler(int signal)
{
  if(signal == SIGTERM){
    cout << "Closing application" << endl;
    app.stop();
  }
}


int main()
{
  signal(SIGTERM, signalHandler);

  app.start();
  cout << "Huenicorn terminated properly" << endl;

  return 0;
}

#pragma once

#include <Huenicorn/IGrabber.hpp>

#include <thread>
#include <optional>
#include <mutex>

#include "XdgDesktopPortal.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include <pipewire/pipewire.h>
#include <spa/param/video/format-utils.h>
#pragma GCC diagnostic pop


namespace Huenicorn
{
  class PipewireGrabber : public IGrabber
  {
  private:
    struct SafeDoubleBuffer
    {
      std::array<cv::Mat, 2> frame;
      std::mutex mutex;
    };


    struct PipewireData
    {
      spa_hook coreListener;
      pw_thread_loop* loop;
      pw_context* context;
      pw_stream* stream;
      spa_video_info format;
      SafeDoubleBuffer frameDoubleBuffer;
      std::promise<bool> ready;
      Config* config;
    };


  public:
    PipewireGrabber(Config* config);

    virtual ~PipewireGrabber();

    virtual Resolution displayResolution() const override;

    virtual RefreshRate displayRefreshRate() const override;

    virtual void grabFrameSubsample(cv::Mat& cvImage) override;


  private:
    // Methods
    static void _onCoreInfoCallback(void* userData, const pw_core_info* info);

    static void _onCoreDoneCallback(void* userData, uint32_t id, int seq);

    static void _onCoreErrorCallback(void* userData, uint32_t id, int seq, int res, const char* message);

    static void _onStreamProcess(void* userdata);

    static void _onStreamParamChanged(void* userdata, uint32_t id, const struct spa_pod* param);

    static void _initCapture(XdgDesktopPortal::Capture* capture);

    static void _pipewireThread(XdgDesktopPortal::Capture* capture, PipewireData* pw);

    void _startPipewireThread();

    void _teardownPipewire();

    void _stop();

    // Attributes
    std::optional<std::thread> m_xdgThread;
    std::optional<std::thread> m_pipewireThread;
    XdgDesktopPortal::Capture m_capture;
    PipewireData m_pwData;
  };
}

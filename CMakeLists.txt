cmake_minimum_required(VERSION 3.0.0)
project(Huenicorn VERSION 0.1.0)


find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  include
)


add_executable(Huenicorn
  src/main.cpp
  src/Channel.cpp
  src/Config.cpp
  src/DtlsClient.cpp
  src/EntertainmentConfigSelector.cpp
  src/HuenicornCore.cpp
  src/ImageProcessing.cpp
  src/JsonCast.cpp
  src/Streamer.cpp
  src/RequestUtils.cpp
  src/SetupBackend.cpp
  src/TickSynchronizer.cpp
  src/WebUIBackend.cpp
  src/X11Grabber.cpp
  include/Huenicorn/Channel.hpp
  include/Huenicorn/Color.hpp
  include/Huenicorn/Config.hpp
  include/Huenicorn/DtlsClient.hpp
  include/Huenicorn/EntertainmentConfig.hpp
  include/Huenicorn/EntertainmentConfigSelector.hpp
  include/Huenicorn/HuenicornCore.hpp
  include/Huenicorn/IGrabber.hpp
  include/Huenicorn/ImageProcessing.hpp
  include/Huenicorn/IRestServer.hpp
  include/Huenicorn/JsonCast.hpp
  include/Huenicorn/Light.hpp
  include/Huenicorn/RequestUtils.hpp
  include/Huenicorn/SetupBackend.hpp
  include/Huenicorn/Streamer.hpp
  include/Huenicorn/TickSynchronizer.hpp
  include/Huenicorn/UV.hpp
  include/Huenicorn/WebUIBackend.hpp
  include/Huenicorn/X11Grabber.hpp
)

set_target_properties(Huenicorn PROPERTIES CXX_STANDARD 20)

target_link_libraries(Huenicorn PUBLIC X11 Xext Xrandr ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp restbed mbedtls mbedcrypto mbedx509)


target_compile_options(Huenicorn PRIVATE
  -Werror
  -Wall
  -Wextra
  -Wnon-virtual-dtor
  -pedantic
  -Wno-deprecated-declarations
  -Wno-deprecated-copy
)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})


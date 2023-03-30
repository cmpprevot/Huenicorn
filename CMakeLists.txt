cmake_minimum_required(VERSION 3.0.0)
project(Huenicorn VERSION 0.1.0)


find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  include
)


set(BinName huenicorn)

add_executable(${BinName}
  src/main.cpp
  src/ApiTools.cpp
  src/Channel.cpp
  src/Config.cpp
  src/Credentials.cpp
  src/DtlsClient.cpp
  src/EntertainmentConfigurationSelector.cpp
  src/HuenicornCore.cpp
  src/ImageProcessing.cpp
  src/JsonSerializer.cpp
  src/Streamer.cpp
  src/RequestUtils.cpp
  src/SetupBackend.cpp
  src/TickSynchronizer.cpp
  src/WebUIBackend.cpp
  src/X11Grabber.cpp
  include/Huenicorn/ApiTools.hpp
  include/Huenicorn/Channel.hpp
  include/Huenicorn/Color.hpp
  include/Huenicorn/Config.hpp
  include/Huenicorn/Credentials.hpp
  include/Huenicorn/Device.hpp
  include/Huenicorn/DtlsClient.hpp
  include/Huenicorn/EntertainmentConfiguration.hpp
  include/Huenicorn/EntertainmentConfigurationSelector.hpp
  include/Huenicorn/HuenicornCore.hpp
  include/Huenicorn/IGrabber.hpp
  include/Huenicorn/ImageProcessing.hpp
  include/Huenicorn/IRestServer.hpp
  include/Huenicorn/JsonSerializer.hpp
  include/Huenicorn/RequestUtils.hpp
  include/Huenicorn/SetupBackend.hpp
  include/Huenicorn/Streamer.hpp
  include/Huenicorn/TickSynchronizer.hpp
  include/Huenicorn/UV.hpp
  include/Huenicorn/WebUIBackend.hpp
  include/Huenicorn/X11Grabber.hpp
)

set_target_properties(${BinName} PROPERTIES CXX_STANDARD 20)

target_link_libraries(${BinName} PUBLIC X11 Xext Xrandr ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp restbed mbedtls mbedcrypto mbedx509)


target_compile_options(${BinName} PRIVATE
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


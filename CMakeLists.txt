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
  src/BridgeData.cpp
  src/Config.cpp
  src/HuenicornCore.cpp
  src/ImageProcessing.cpp
  src/SyncedLight.cpp
  src/RequestUtils.cpp
  src/SetupBackend.cpp
  src/ScreenUtils.cpp
  src/TickSynchronizer.cpp
  src/WebUIBackend.cpp
  include/Huenicorn/BridgeData.hpp
  include/Huenicorn/Color.hpp
  include/Huenicorn/Config.hpp
  include/Huenicorn/HuenicornCore.hpp
  include/Huenicorn/ImageProcessing.hpp
  include/Huenicorn/ImageData.hpp
  include/Huenicorn/IRestServer.hpp
  include/Huenicorn/SyncedLight.hpp
  include/Huenicorn/LightSummary.hpp
  include/Huenicorn/RequestUtils.hpp
  include/Huenicorn/SetupBackend.hpp
  include/Huenicorn/ScreenUtils.hpp
  include/Huenicorn/TickSynchronizer.hpp
  include/Huenicorn/WebUIBackend.hpp
)

set_target_properties(Huenicorn PROPERTIES CXX_STANDARD 20)

target_link_libraries(Huenicorn PUBLIC X11 ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp restbed)


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


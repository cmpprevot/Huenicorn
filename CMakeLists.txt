cmake_minimum_required(VERSION 3.0.0)
project(FreenSync VERSION 0.1.0)


find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  include
)

add_executable(FreenSync
  src/main.cpp
  src/BridgeData.cpp
  src/Communicator.cpp
  src/ImageProcessor.cpp
  src/Light.cpp
  include/FreenSync/BridgeData.hpp
  include/FreenSync/Communicator.hpp
  include/FreenSync/ImageProcessor.hpp
  include/FreenSync/Light.hpp
)

set_target_properties(FreenSync PROPERTIES CXX_STANDARD 20)

target_link_libraries(FreenSync PUBLIC X11 ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

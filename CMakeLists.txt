cmake_minimum_required(VERSION 3.0.0)
project(FreenSync VERSION 0.1.0)


find_package( OpenCV REQUIRED )
include_directories(
  ${OpenCV_INCLUDE_DIRS}
  include
)

add_executable(FreenSync
  src/main.cpp
  src/ImageProcessor.cpp
  include/FreenSync/ImageProcessor.hpp
)

target_link_libraries(FreenSync PUBLIC X11 ${OpenCV_LIBS})

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

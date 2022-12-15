cmake_minimum_required(VERSION 3.0.0)
project(FreenSync VERSION 0.1.0)


find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

include_directories(
  ${OpenCV_INCLUDE_DIRS}
  include
  ext/restbed/source
)


add_subdirectory(ext/restbed)
link_directories(build/ext/restbed) # ToDo : Fix path

add_executable(FreenSync
  src/main.cpp
  src/BridgeData.cpp
  src/Communicator.cpp
  src/FreenSync.cpp
  src/ImageProcessing.cpp
  src/SyncedLight.cpp
  src/RestServer.cpp
  src/ScreenUtils.cpp
  src/TickSynchronizer.cpp
  include/FreenSync/BridgeData.hpp
  include/FreenSync/Color.hpp
  include/FreenSync/Communicator.hpp
  include/FreenSync/FreenSync.hpp
  include/FreenSync/ImageProcessing.hpp
  include/FreenSync/ImageData.hpp
  include/FreenSync/SyncedLight.hpp
  include/FreenSync/LightSummary.hpp
  include/FreenSync/RestServer.hpp
  include/FreenSync/ScreenUtils.hpp
  include/FreenSync/TickSynchronizer.hpp
)

set_target_properties(FreenSync PROPERTIES CXX_STANDARD 20)

target_link_libraries(FreenSync PUBLIC X11 ${OpenCV_LIBS} ${CURL_LIBRARIES} curlpp restbed)


target_compile_options(FreenSync PRIVATE
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


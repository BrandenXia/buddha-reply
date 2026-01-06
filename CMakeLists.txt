cmake_minimum_required(VERSION 3.16)
project(buddha-reply)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
add_executable(buddha-reply src/main.cpp src/msg.cpp src/bpe.cpp)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp)
include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/SQLiteCpp/include)
target_link_libraries(buddha-reply
  SQLiteCpp
  sqlite3
  pthread
  dl
)

include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/cppcoro/include)

include_directories(${CMAKE_CURRENT_LIST_DIR}/thirdparty/rapidjson/include)

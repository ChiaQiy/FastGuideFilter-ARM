cmake_minimum_required(VERSION 3.5)

project(demo)

set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -mfpu=neon")

include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(a.out)
file(GLOB sources CONFIGURE_DEPENDS src/*.cpp main.cpp)
target_sources(a.out PUBLIC ${sources})

cmake_minimum_required(VERSION 3.20)
project(inf_theory_2)

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libstdc++ -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ,--no-whole-archive")
set(EXECUTABLE_OUTPUT_PATH ../bin )

add_executable(coder src/coder.cpp src/lib.h)
add_executable(decoder src/decoder.cpp src/lib.h)

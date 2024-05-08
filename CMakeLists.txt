cmake_minimum_required(VERSION 3.5)

project(exampleLib CXX ASM)

set(CMAKE_CXX_STANDARD 20)

pico_sdk_init()

add_library( exampleLib
    example.hpp
    example.cpp
    )

target_link_libraries( exampleLib
  pico_stdlib 
  
  # Using Pico W
  pico_cyw43_arch_none
)

target_include_directories(exampleLib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
